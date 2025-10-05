#include "gl_bridge.hpp"
#include "egl_wrapper.hpp"
#include "../../../gl/src/gl_renderer.hpp"

namespace {
    EGLContextWrapper g_egl;
    edgegl::GLRenderer g_renderer;
    bool g_initialized = false;
}

namespace edgeviewer_gl_jni {

bool initWithSurface(JNIEnv* env, jobject surface) {
    if (g_initialized) return true;
    if (!g_egl.initializeFromSurface(env, surface)) return false;
    if (!g_renderer.initialize()) return false;
    g_initialized = true;
    return true;
}

bool renderFrame() {
    if (!g_initialized) return false;
    g_egl.makeCurrent();
    g_renderer.renderFrame();
    g_egl.swapBuffers();
    return true;
}

void resize(int width, int height) {
    if (!g_initialized) return;
    g_renderer.resize(width, height);
}

bool uploadGrayTexture(const uint8_t* data, int width, int height) {
    if (!g_initialized) return false;
    g_egl.makeCurrent();
    return g_renderer.uploadGrayTexture(data, width, height);
}

void shutdown() {
    if (!g_initialized) return;
    g_initialized = false;
    g_egl.shutdown();
}

}


