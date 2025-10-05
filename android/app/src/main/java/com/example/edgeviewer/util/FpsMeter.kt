package com.example.edgeviewer.util

class FpsMeter(private val windowMs: Long = 1000L) {
    private var lastTime = System.nanoTime()
    private var frameCount = 0
    private var fps = 0.0

    fun tick(): Double {
        frameCount++
        val now = System.nanoTime()
        val dtMs = (now - lastTime) / 1_000_000.0
        if (dtMs >= windowMs) {
            fps = frameCount * (1000.0 / dtMs)
            frameCount = 0
            lastTime = now
        }
        return fps
    }

    fun getFps(): Double = fps
}


