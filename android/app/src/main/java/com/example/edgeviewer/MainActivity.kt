package com.example.edgeviewer

import android.Manifest
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.widget.TextView
import android.view.TextureView
import android.util.Size
import com.example.edgeviewer.camera.Camera2Controller
import com.example.edgeviewer.processing.FrameProcessor
import java.nio.ByteBuffer
import androidx.activity.ComponentActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat

class MainActivity : ComponentActivity() {

    companion object {
        init {
            System.loadLibrary("edgeviewer")
        }
    }

    external fun stringFromJNI(): String

    private lateinit var cameraController: Camera2Controller

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val statusText = findViewById<TextView>(R.id.statusText)
        statusText.text = stringFromJNI()

        cameraController = Camera2Controller(this)

        ensurePermissions()
        tryStartCamera()

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
            cameraController.openBackCamera(
                onOpened = {
                    runOnUiThread { statusText.text = "Camera opened" }
                    cameraController.startPreview(textureView, Size(textureView.width, textureView.height))
                },
                onError = { code ->
                    runOnUiThread { statusText.text = "Camera error: $code" }
                }
            )
        }
    }

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
        cameraController.close()
        cameraController.stopBackgroundThread()
    }
}


