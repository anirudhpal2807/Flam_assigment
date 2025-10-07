package com.example.edgeviewer.processing

import android.graphics.ImageFormat
import android.media.Image

object YuvUtils {

    fun yuv420ToRgba(image: Image, outRgba: ByteArray): Boolean {
        if (image.format != ImageFormat.YUV_420_888) return false
        val width = image.width
        val height = image.height
        if (outRgba.size < width * height * 4) return false

        val yPlane = image.planes[0]
        val uPlane = image.planes[1]
        val vPlane = image.planes[2]

        val yBuffer = yPlane.buffer
        val uBuffer = uPlane.buffer
        val vBuffer = vPlane.buffer

        val yRowStride = yPlane.rowStride
        val yPixelStride = yPlane.pixelStride
        val uRowStride = uPlane.rowStride
        val uPixelStride = uPlane.pixelStride
        val vRowStride = vPlane.rowStride
        val vPixelStride = vPlane.pixelStride

        var out = 0
        for (j in 0 until height) {
            val yRow = j * yRowStride
            val uvRow = (j shr 1) * uRowStride
            for (i in 0 until width) {
                val yIndex = yRow + i * yPixelStride
                val uIndex = uvRow + (i shr 1) * uPixelStride
                val vIndex = (j shr 1) * vRowStride + (i shr 1) * vPixelStride

                val y = yBuffer.get(yIndex).toInt() and 0xFF
                val u = uBuffer.get(uIndex).toInt() and 0xFF
                val v = vBuffer.get(vIndex).toInt() and 0xFF

                val yf = (y - 16).coerceAtLeast(0)
                val uf = u - 128
                val vf = v - 128

                var r = (1192 * yf + 1634 * vf) shr 10
                var g = (1192 * yf - 833 * vf - 400 * uf) shr 10
                var b = (1192 * yf + 2066 * uf) shr 10

                if (r < 0) r = 0 else if (r > 255) r = 255
                if (g < 0) g = 0 else if (g > 255) g = 255
                if (b < 0) b = 0 else if (b > 255) b = 255

                // RGBA
                outRgba[out++] = r.toByte()
                outRgba[out++] = g.toByte()
                outRgba[out++] = b.toByte()
                outRgba[out++] = (-1).toByte()
            }
        }
        return true
    }
}


