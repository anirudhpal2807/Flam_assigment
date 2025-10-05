#include "egl_wrapper.hpp"

#include <android/native_window_jni.h>

static EGLConfig chooseConfig(EGLDisplay display) {
    const EGLint attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RED_SIZE,        8,
        EGL_GREEN_SIZE,      8,
        EGL_BLUE_SIZE,       8,
        EGL_ALPHA_SIZE,      8,
        EGL_DEPTH_SIZE,      0,
        EGL_STENCIL_SIZE,    0,
        EGL_NONE
    };
    EGLConfig config = nullptr;
    EGLint num = 0;
    eglChooseConfig(display, attribs, &config, 1, &num);
    return (num > 0) ? config : nullptr;
}

bool EGLContextWrapper::initializeFromSurface(JNIEnv* env, jobject surfaceObj) {
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) return false;

    if (!eglInitialize(display, nullptr, nullptr)) return false;
    EGLConfig config = chooseConfig(display);
    if (!config) return false;

    ANativeWindow* window = ANativeWindow_fromSurface(env, surfaceObj);
    if (!window) return false;

    surface = eglCreateWindowSurface(display, config, window, nullptr);
    ANativeWindow_release(window);
    if (surface == EGL_NO_SURFACE) return false;

    const EGLint ctxAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxAttribs);
    if (context == EGL_NO_CONTEXT) return false;

    if (!eglMakeCurrent(display, surface, surface, context)) return false;
    return true;
}

void EGLContextWrapper::makeCurrent() {
    if (display != EGL_NO_DISPLAY && surface != EGL_NO_SURFACE && context != EGL_NO_CONTEXT) {
        eglMakeCurrent(display, surface, surface, context);
    }
}

void EGLContextWrapper::swapBuffers() {
    if (display != EGL_NO_DISPLAY && surface != EGL_NO_SURFACE) {
        eglSwapBuffers(display, surface);
    }
}

void EGLContextWrapper::shutdown() {
    if (display != EGL_NO_DISPLAY) {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (context != EGL_NO_CONTEXT) {
            eglDestroyContext(display, context);
            context = EGL_NO_CONTEXT;
        }
        if (surface != EGL_NO_SURFACE) {
            eglDestroySurface(display, surface);
            surface = EGL_NO_SURFACE;
        }
        eglTerminate(display);
        display = EGL_NO_DISPLAY;
    }
}


