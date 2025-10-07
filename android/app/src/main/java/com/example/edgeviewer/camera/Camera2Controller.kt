package com.example.edgeviewer.camera

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.SurfaceTexture
import android.hardware.camera2.*
import android.os.Handler
import android.os.HandlerThread
import android.util.Size
import android.view.Surface
import android.view.TextureView
import android.media.ImageReader

class Camera2Controller(private val context: Context) {

    private val cameraManager: CameraManager = context.getSystemService(Context.CAMERA_SERVICE) as CameraManager
    private var cameraDevice: CameraDevice? = null
    private var captureSession: CameraCaptureSession? = null
    private var backgroundThread: HandlerThread? = null
    private var backgroundHandler: Handler? = null

    fun startBackgroundThread() {
        backgroundThread = HandlerThread("CameraBackground").also { it.start() }
        backgroundHandler = Handler(backgroundThread!!.looper)
    }

    fun stopBackgroundThread() {
        backgroundThread?.quitSafely()
        try {
            backgroundThread?.join()
        } catch (_: InterruptedException) {}
        backgroundThread = null
        backgroundHandler = null
    }

    fun getBackgroundHandler(): Handler? = backgroundHandler

    fun setUpTextureView(textureView: TextureView, onReady: () -> Unit) {
        if (textureView.isAvailable) {
            onReady()
        } else {
            textureView.surfaceTextureListener = object : TextureView.SurfaceTextureListener {
                override fun onSurfaceTextureAvailable(surface: SurfaceTexture, width: Int, height: Int) {
                    onReady()
                }
                override fun onSurfaceTextureSizeChanged(surface: SurfaceTexture, width: Int, height: Int) {}
                override fun onSurfaceTextureDestroyed(surface: SurfaceTexture): Boolean = true
                override fun onSurfaceTextureUpdated(surface: SurfaceTexture) {}
            }
        }
    }

    @SuppressLint("MissingPermission")
    fun openBackCamera(onOpened: () -> Unit, onError: (Int) -> Unit) {
        val cameraId = cameraManager.cameraIdList.firstOrNull { id ->
            val chars = cameraManager.getCameraCharacteristics(id)
            chars.get(CameraCharacteristics.LENS_FACING) == CameraCharacteristics.LENS_FACING_BACK
        } ?: cameraManager.cameraIdList.first()

        cameraManager.openCamera(cameraId, object : CameraDevice.StateCallback() {
            override fun onOpened(camera: CameraDevice) {
                cameraDevice = camera
                onOpened()
            }
            override fun onDisconnected(camera: CameraDevice) {
                camera.close(); cameraDevice = null
            }
            override fun onError(camera: CameraDevice, error: Int) {
                camera.close(); cameraDevice = null; onError(error)
            }
        }, backgroundHandler)
    }

    @SuppressLint("MissingPermission")
    fun openBackCameraWithImageReader(onOpened: () -> Unit, onError: (Int) -> Unit) {
        openBackCamera(onOpened, onError)
    }

    fun startPreview(textureView: TextureView, desiredSize: Size? = null) {
        val device = cameraDevice ?: return
        val texture = textureView.surfaceTexture ?: return

        if (desiredSize != null) {
            texture.setDefaultBufferSize(desiredSize.width, desiredSize.height)
        }
        val surface = Surface(texture)

        val requestBuilder = device.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW).apply {
            addTarget(surface)
        }

        device.createCaptureSession(listOf(surface), object : CameraCaptureSession.StateCallback() {
            override fun onConfigured(session: CameraCaptureSession) {
                captureSession = session
                requestBuilder.set(CaptureRequest.CONTROL_MODE, CameraMetadata.CONTROL_MODE_AUTO)
                val request = requestBuilder.build()
                session.setRepeatingRequest(request, null, backgroundHandler)
            }
            override fun onConfigureFailed(session: CameraCaptureSession) {}
        }, backgroundHandler)
    }

    fun startImageSession(imageReader: ImageReader) {
        val device = cameraDevice ?: return
        val surface = imageReader.surface

        val requestBuilder = device.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW).apply {
            addTarget(surface)
        }

        device.createCaptureSession(listOf(surface), object : CameraCaptureSession.StateCallback() {
            override fun onConfigured(session: CameraCaptureSession) {
                captureSession = session
                requestBuilder.set(CaptureRequest.CONTROL_MODE, CameraMetadata.CONTROL_MODE_AUTO)
                val request = requestBuilder.build()
                session.setRepeatingRequest(request, null, backgroundHandler)
            }
            override fun onConfigureFailed(session: CameraCaptureSession) {}
        }, backgroundHandler)
    }

    fun close() {
        captureSession?.close(); captureSession = null
        cameraDevice?.close(); cameraDevice = null
    }
}


