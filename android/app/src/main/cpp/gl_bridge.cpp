#include "gl_bridge.hpp"

namespace edgeviewer_gl_jni {

bool initWithSurface(JNIEnv* /*env*/, jobject /*surface*/) {
    // TODO: Wire up EGL with ANativeWindow from Surface and use edgegl::GLRenderer
    return true; // no-op success for scaffold
}

bool renderFrame() {
    // TODO: Call renderer.renderFrame() once wired
    return true;
}

void resize(int /*width*/, int /*height*/) {
    // TODO: Forward to renderer.resize
}

void shutdown() {
    // TODO: Destroy EGL and GL resources
}

}


