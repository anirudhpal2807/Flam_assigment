package com.example.edgeviewer

import android.view.Surface

object GLBridge {
    init {
        System.loadLibrary("edgeviewer")
    }

    external fun initWithSurface(surface: Surface): Boolean
    external fun renderFrame(): Boolean
    external fun resize(width: Int, height: Int)
    external fun shutdown()

    external fun uploadGrayTexture(buffer: java.nio.ByteBuffer, width: Int, height: Int): Boolean
}


