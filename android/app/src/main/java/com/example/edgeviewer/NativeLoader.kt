package com.example.edgeviewer

import java.util.concurrent.atomic.AtomicBoolean

object NativeLoader {
    private val loaded = AtomicBoolean(false)

    fun ensureLoaded(onLoaded: (() -> Unit)? = null) {
        if (loaded.get()) {
            onLoaded?.let { it() }
            return
        }
        Thread {
            try {
                System.loadLibrary("edgeviewer")
                loaded.set(true)
                onLoaded?.let { it() }
            } catch (_: Throwable) {
                // ignore; calls will fail gracefully if not loaded
            }
        }.start()
    }

    fun isLoaded(): Boolean = loaded.get()
}


