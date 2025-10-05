#pragma once

#include <jni.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

// Minimal EGL wrapper to create/destroy context and swap buffers.
// Implementation will be added in a later commit.
struct EGLContextWrapper {
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLSurface surface = EGL_NO_SURFACE;
    EGLContext context = EGL_NO_CONTEXT;

    bool initializeFromSurface(JNIEnv* env, jobject surface /* android.view.Surface */);
    void makeCurrent();
    void swapBuffers();
    void shutdown();
};


