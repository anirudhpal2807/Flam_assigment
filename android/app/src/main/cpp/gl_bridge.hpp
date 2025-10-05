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

// Upload a grayscale image as texture for rendering
bool uploadGrayTexture(const uint8_t* data, int width, int height);

// Destroy GL resources
void shutdown();

}


