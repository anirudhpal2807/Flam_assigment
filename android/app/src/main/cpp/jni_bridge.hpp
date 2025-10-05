#pragma once

#include <jni.h>
#include <cstdint>
#include <cstddef>

namespace edgeviewer_jni {

// Initialize any native singletons/resources if needed
void init(JNIEnv* env, jobject context);

// Process an RGBA image buffer to grayscale; returns a direct ByteBuffer (read-only) if succeed,
// or null on failure. The buffer is owned by native code and is valid until the next call.
jobject processGrayscale(JNIEnv* env,
                         const uint8_t* rgba,
                         int width,
                         int height,
                         int strideBytes);

// Process with Canny and return a direct ByteBuffer (single-channel mask)
jobject processCanny(JNIEnv* env,
                    const uint8_t* rgba,
                    int width,
                    int height,
                    int strideBytes,
                    double lowThresh,
                    double highThresh);

}


