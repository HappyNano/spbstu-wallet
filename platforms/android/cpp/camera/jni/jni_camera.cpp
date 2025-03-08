#include "jni_camera.h"

#include <android/log.h>
#include <cstring>

#include <platforms/android/cpp/camera/camera.h>

extern "C" JNIEXPORT void JNICALL
 Java_imgui_example_android_CameraHelper_onImageAvailable(JNIEnv * env, jclass, jbyteArray data, jint width, jint height) {
    static constexpr int CHANNELS = 4;

    __android_log_print(ANDROID_LOG_INFO, "CameraHelperJni", "%s", "New data avaible");

    jbyte * yuvData = env->GetByteArrayElements(data, nullptr);

    auto * newData = new jbyte[width * height * CHANNELS];
    memcpy(newData, yuvData, sizeof(jbyte) * width * height * CHANNELS);
    env->ReleaseByteArrayElements(data, yuvData, JNI_ABORT);

    cxx::CameraHelper::get()->newTexture(newData, width, height, CHANNELS);
}
