package com.example.edgeviewer

object NativeBridge {
    init {
        System.loadLibrary("edgeviewer")
    }

    external fun processGrayscale(
        rgbaBuffer: ByteArray,
        width: Int,
        height: Int,
        strideBytes: Int
    ): java.nio.ByteBuffer?

    external fun processCanny(
        rgbaBuffer: ByteArray,
        width: Int,
        height: Int,
        strideBytes: Int,
        lowThresh: Double,
        highThresh: Double
    ): java.nio.ByteBuffer?
}


