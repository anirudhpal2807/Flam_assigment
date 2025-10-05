#pragma once

#include <jni.h>

namespace edgeviewer_gl_jni {

// Initialize GL with a Java Surface (ANativeWindow under the hood)
// Returns true on success.
bool initWithSurface(JNIEnv* env, jobject surface);

// Render a frame (no-op until wired). Returns true if drew successfully.
bool renderFrame();

// Resize viewport
void resize(int width, int height);

// Destroy GL resources
void shutdown();

}


