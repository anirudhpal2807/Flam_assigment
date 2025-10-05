#include <jni.h>
#include <string>
#include "jni_bridge.hpp"

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_edgeviewer_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++ (NDK scaffold)";
    return env->NewStringUTF(hello.c_str());
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


