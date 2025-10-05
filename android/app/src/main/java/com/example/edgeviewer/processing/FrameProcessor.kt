package com.example.edgeviewer.processing

import android.graphics.Bitmap
import android.view.TextureView
import com.example.edgeviewer.NativeBridge
import java.nio.ByteBuffer

object FrameProcessor {

    fun captureRgba(textureView: TextureView): ByteArray? {
        if (!textureView.isAvailable) return null
        val bmp = textureView.bitmap ?: return null
        val rgba = bitmapToRgba(bmp)
        bmp.recycle()
        return rgba
    }

    fun toGrayscale(bufferRgba: ByteArray, width: Int, height: Int, strideBytes: Int): ByteBuffer? {
        return NativeBridge.processGrayscale(bufferRgba, width, height, strideBytes)
    }

    private fun bitmapToRgba(bitmap: Bitmap): ByteArray {
        val width = bitmap.width
        val height = bitmap.height
        val bytes = ByteArray(width * height * 4)
        val buf = ByteBuffer.wrap(bytes)
        bitmap.copyPixelsToBuffer(buf)
        // Bitmap default config from TextureView is ARGB_8888, convert to RGBA by swapping R and B
        for (i in 0 until width * height) {
            val base = i * 4
            val a = bytes[base + 3]
            val r = bytes[base + 2]
            val g = bytes[base + 1]
            val b = bytes[base + 0]
            bytes[base + 0] = r
            bytes[base + 1] = g
            bytes[base + 2] = b
            bytes[base + 3] = a
        }
        return bytes
    }
}


