#include <jni.h>
#include <string>
#include "jni_bridge.hpp"
#include "gl_bridge.hpp"

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_edgeviewer_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++ (NDK scaffold)";
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_edgeviewer_GLBridge_initWithSurface(
        JNIEnv* env,
        jobject /* thiz */,
        jobject surface) {
    return edgeviewer_gl_jni::initWithSurface(env, surface) ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_edgeviewer_GLBridge_renderFrame(
        JNIEnv* /* env */,
        jobject /* thiz */) {
    return edgeviewer_gl_jni::renderFrame() ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_edgeviewer_GLBridge_resize(
        JNIEnv* /* env */,
        jobject /* thiz */,
        jint width,
        jint height) {
    edgeviewer_gl_jni::resize(width, height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_edgeviewer_GLBridge_shutdown(
        JNIEnv* /* env */,
        jobject /* thiz */) {
    edgeviewer_gl_jni::shutdown();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_edgeviewer_GLBridge_uploadGrayTexture(
        JNIEnv* env,
        jobject /* thiz */,
        jobject buffer,
        jint width,
        jint height) {
    uint8_t* data = static_cast<uint8_t*>(env->GetDirectBufferAddress(buffer));
    if (!data) return JNI_FALSE;
    return edgeviewer_gl_jni::uploadGrayTexture(data, width, height) ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_example_edgeviewer_NativeBridge_processGrayscale(
        JNIEnv* env,
        jobject /* thiz */,
        jbyteArray rgbaBuffer,
        jint width,
        jint height,
        jint strideBytes) {
    jsize len = env->GetArrayLength(rgbaBuffer);
    jbyte* ptr = env->GetByteArrayElements(rgbaBuffer, nullptr);
    jobject result = edgeviewer_jni::processGrayscale(env, reinterpret_cast<const uint8_t*>(ptr), width, height, strideBytes);
    env->ReleaseByteArrayElements(rgbaBuffer, ptr, JNI_ABORT);
    return result;
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_example_edgeviewer_NativeBridge_processCanny(
        JNIEnv* env,
        jobject /* thiz */,
        jbyteArray rgbaBuffer,
        jint width,
        jint height,
        jint strideBytes,
        jdouble lowThresh,
        jdouble highThresh) {
    jbyte* ptr = env->GetByteArrayElements(rgbaBuffer, nullptr);
    jobject result = edgeviewer_jni::processCanny(env, reinterpret_cast<const uint8_t*>(ptr), width, height, strideBytes, lowThresh, highThresh);
    env->ReleaseByteArrayElements(rgbaBuffer, ptr, JNI_ABORT);
    return result;
}


