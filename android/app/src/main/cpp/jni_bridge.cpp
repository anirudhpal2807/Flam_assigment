#include "jni_bridge.hpp"
#include "../../../../../jni/src/opencv_pipeline.hpp"

#include <vector>
#include <android/log.h>

#define LOG_TAG "EdgeViewerJNI"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace {
    std::vector<uint8_t> g_grayBuffer; // reused across calls
}

namespace edgeviewer_jni {

void init(JNIEnv* /*env*/, jobject /*context*/) {
    g_grayBuffer.clear();
}

jobject processGrayscale(JNIEnv* env,
                         const uint8_t* rgba,
                         int width,
                         int height,
                         int strideBytes) {
    if (!rgba || width <= 0 || height <= 0) return nullptr;

    edgeviewer::ImageView src{rgba, width, height, strideBytes, 4};
    size_t outBytes = 0;
    if (!edgeviewer::processGrayscale(src, nullptr, 0, outBytes)) {
        g_grayBuffer.resize(outBytes);
    }
    size_t written = 0;
    if (!edgeviewer::processGrayscale(src, g_grayBuffer.data(), g_grayBuffer.size(), written)) {
        LOGE("processGrayscale failed even after buffer alloc");
        return nullptr;
    }

    return env->NewDirectByteBuffer(g_grayBuffer.data(), static_cast<jlong>(written));
}

jobject processCanny(JNIEnv* env,
                    const uint8_t* rgba,
                    int width,
                    int height,
                    int strideBytes,
                    double lowThresh,
                    double highThresh) {
    if (!rgba || width <= 0 || height <= 0) return nullptr;

    edgeviewer::ImageView src{rgba, width, height, strideBytes, 4};
    size_t outBytes = 0;
    if (!edgeviewer::processCannyEdges(src, lowThresh, highThresh, nullptr, 0, outBytes)) {
        g_grayBuffer.resize(outBytes);
    }
    size_t written = 0;
    if (!edgeviewer::processCannyEdges(src, lowThresh, highThresh, g_grayBuffer.data(), g_grayBuffer.size(), written)) {
        LOGE("processCanny failed even after buffer alloc");
        return nullptr;
    }

    return env->NewDirectByteBuffer(g_grayBuffer.data(), static_cast<jlong>(written));
}

} // namespace edgeviewer_jni


