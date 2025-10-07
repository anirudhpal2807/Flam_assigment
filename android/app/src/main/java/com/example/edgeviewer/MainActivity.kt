package com.example.edgeviewer

import android.Manifest
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.widget.TextView
import android.view.TextureView
import android.util.Size
import android.media.ImageReader
import android.graphics.ImageFormat
import com.example.edgeviewer.camera.Camera2Controller
import com.example.edgeviewer.processing.FrameProcessor
import com.example.edgeviewer.util.FpsMeter
import android.os.Looper
import android.os.Handler
import android.view.Surface
import android.widget.Button
import java.nio.ByteBuffer
import androidx.activity.ComponentActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
 
import java.lang.Runnable
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import java.io.ByteArrayOutputStream
import java.net.HttpURLConnection
import java.net.URL

class MainActivity : ComponentActivity() {

    external fun stringFromJNI(): String

    private lateinit var cameraController: Camera2Controller
    private var renderHandler: Handler? = null
    private val fpsMeter = FpsMeter()
    private var rendering = false
    private var useCanny = true
    private var frameCounter = 0
    private var imageReader: ImageReader? = null
    private var rgbaBuffer: ByteArray? = null
    private var lastFrameW: Int = 0
    private var lastFrameH: Int = 0

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Keep existing layout; ensure it has a TextureView with id "texture_view"
        setContentView(R.layout.activity_main)

        val statusText = findViewById<TextView>(R.id.statusText)
        statusText.text = "Starting..."

        NativeLoader.ensureLoaded {
            runOnUiThread {
                try {
                    statusText.text = stringFromJNI()
                } catch (_: Throwable) {
                    statusText.text = "Native ready"
                }
                tryStartCamera()
            }
        }

        cameraController = Camera2Controller(this)

        ensurePermissions()

        // Tap status to process one frame via JNI (grayscale) and show stats
        statusText.setOnClickListener {
            val textureView = findViewById<TextureView>(R.id.textureView)
            val rgba = FrameProcessor.captureRgba(textureView)
            if (rgba == null) {
                statusText.text = "No frame"
                return@setOnClickListener
            }
            val w = textureView.width
            val h = textureView.height
            val stride = w * 4
            val t0 = System.nanoTime()
            val gray: ByteBuffer? = FrameProcessor.toGrayscale(rgba, w, h, stride)
            val dtMs = (System.nanoTime() - t0) / 1_000_000.0
            val size = gray?.capacity() ?: 0
            if (gray != null) {
                val uri = com.example.edgeviewer.processing.ImageSaver.saveGrayscalePng(this, gray, w, h)
                statusText.text = "Saved: ${uri ?: "(failed)"}\n${size}B | ${w}x${h} | ${"%.1f".format(dtMs)}ms"
            } else {
                statusText.text = "Gray failed | ${w}x${h} | ${"%.1f".format(dtMs)}ms"
            }
        }

        // Default to CANNY mode; hide toggle for auto-run experience
        val toggleButton = findViewById<Button>(R.id.toggleButton)
        toggleButton.text = "Mode: CANNY"
        toggleButton.visibility = android.view.View.GONE
    }

    private fun ensurePermissions() {
        val needsCamera = ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED
        if (needsCamera) {
            val perms = arrayOf(Manifest.permission.CAMERA)
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                ActivityCompat.requestPermissions(this, perms, 100)
            }
        }
    }

    private fun tryStartCamera() {
        val hasCamera = ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED
        if (!hasCamera) return

        val textureView = findViewById<TextureView>(R.id.textureView)
        val statusText = findViewById<TextView>(R.id.statusText)

        cameraController.startBackgroundThread()
        cameraController.setUpTextureView(textureView) {
            if (!NativeLoader.isLoaded()) return@setUpTextureView
            cameraController.openBackCamera(
                onOpened = {
                    runOnUiThread { statusText.text = "Camera opened" }

                    val w = if (textureView.width > 0) textureView.width else 1280
                    val h = if (textureView.height > 0) textureView.height else 720
                    imageReader = ImageReader.newInstance(w, h, ImageFormat.YUV_420_888, 3)
                    rgbaBuffer = ByteArray(w * h * 4)
                    lastFrameW = w
                    lastFrameH = h

                    imageReader?.setOnImageAvailableListener({ reader ->
                        val image = reader.acquireLatestImage() ?: return@setOnImageAvailableListener
                        // Realloc and resize GL if frame size changes (some devices adjust stream size)
                        val width = image.width
                        val height = image.height
                        if (width != lastFrameW || height != lastFrameH) {
                            rgbaBuffer = ByteArray(width * height * 4)
                            lastFrameW = width
                            lastFrameH = height
                            try { GLBridge.resize(width, height) } catch (_: Throwable) {}
                        }
                        val rgba = rgbaBuffer ?: run { image.close(); return@setOnImageAvailableListener }
                        val ok = com.example.edgeviewer.processing.YuvUtils.yuv420ToRgba(image, rgba)
                        image.close()
                        if (ok) {
                            val buffer: ByteBuffer? = try {
                                // Stronger thresholds to match the crisp web result
                                NativeBridge.processCanny(rgba, width, height, width * 4, 80.0, 200.0)
                            } catch (_: Throwable) { null }
                            if (buffer != null) {
                                try { GLBridge.uploadGrayTexture(buffer, width, height) } catch (_: Throwable) {}
                            }
                        }
                    }, cameraController.getBackgroundHandler())

                    val readerSurface = imageReader!!.surface

                    // Initialize GL on the TextureView surface for display
                    val surf = Surface(textureView.surfaceTexture)
                    if (GLBridge.initWithSurface(surf)) {
                        GLBridge.resize(w, h)
                    }

                    cameraController.startImageSession(imageReader!!)
                    startRenderLoop(statusText)
                },
                onError = { code ->
                    runOnUiThread { statusText.text = "Camera error: $code" }
                }
            )
        }
    }

    private fun startRenderLoop(statusText: TextView) {
        if (rendering) return
        rendering = true
        if (renderHandler == null) renderHandler = Handler(Looper.getMainLooper())
        val loop = object : Runnable {
            override fun run() {
                if (!rendering) return
                // Try to grab a frame from TextureView, process edges/gray via JNI, upload to GL
                val textureView = findViewById<TextureView>(R.id.textureView)
                val w = textureView.width
                val h = textureView.height
                if (w > 0 && h > 0) {
                    frameCounter += 1
                    val rgba = FrameProcessor.captureRgba(textureView)
                    if (rgba != null) {
                        // Occasionally send a JPEG to the local web server (every ~60 frames)
                        if (frameCounter % 60 == 0) {
                            try { sendFrameToWeb(textureView) } catch (_: Exception) {}
                        }
                        val buffer: ByteBuffer? = try {
                            NativeBridge.processCanny(rgba, w, h, w * 4, 50.0, 150.0)
                        } catch (_: Throwable) {
                            null
                        }
                        if (buffer != null) {
                            try { GLBridge.uploadGrayTexture(buffer, w, h) } catch (_: Throwable) {}
                        }
                    }
                }
                try { GLBridge.renderFrame() } catch (_: Throwable) {}
                val fps = fpsMeter.tick()
                if (fps > 0.0) statusText.text = "FPS: ${"%.1f".format(fps)}"
                renderHandler?.postDelayed(this, 16L)
            }
        }
        renderHandler?.post(loop)
    }

    private fun sendFrameToWeb(textureView: TextureView) {
        // Capture bitmap directly from the TextureView and compress to JPEG
        val bmp = textureView.bitmap ?: return
        val baos = ByteArrayOutputStream()
        bmp.compress(Bitmap.CompressFormat.JPEG, 60, baos)
        bmp.recycle()
        val jpeg = baos.toByteArray()

        Thread {
            try {
                // If you changed server port, update it here to match server console output
                val url = URL("http://10.0.2.2:5173/ingest")
                val conn = (url.openConnection() as HttpURLConnection).apply {
                    requestMethod = "POST"
                    doOutput = true
                    setRequestProperty("Content-Type", "image/jpeg")
                    connectTimeout = 1500
                    readTimeout = 1500
                }
                conn.outputStream.use { it.write(jpeg) }
                try { conn.inputStream.close() } catch (_: Exception) {}
                try { conn.disconnect() } catch (_: Exception) {}
            } catch (_: Exception) {}
        }.start()
    }

    @Suppress("DEPRECATION")
    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == 100 && grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            tryStartCamera()
        }
    }

    override fun onResume() {
        super.onResume()
        tryStartCamera()
    }

    override fun onPause() {
        super.onPause()
        rendering = false
        renderHandler?.removeCallbacksAndMessages(null)
        cameraController.close()
        cameraController.stopBackgroundThread()
    }

    override fun onDestroy() {
        super.onDestroy()
        try { GLBridge.shutdown() } catch (_: Throwable) {}
    }
}


