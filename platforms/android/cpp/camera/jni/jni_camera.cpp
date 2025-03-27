#include "jni_camera.h"

#include <spdlog/spdlog.h>

#include <platforms/android/cpp/camera/camera.h>

#include <cstring>

extern "C" JNIEXPORT void JNICALL
 Java_imgui_example_android_CameraHelper_onImageAvailable(JNIEnv * env, jclass, jbyteArray data, jint width, jint height) {
    static constexpr int CHANNELS = 4;

    SPDLOG_INFO("JNI: onImageAvailable: %s", "New data avaible");

    jbyte * yuvData = env->GetByteArrayElements(data, nullptr);

    auto * newData = new jbyte[width * height * CHANNELS];
    memcpy(newData, yuvData, sizeof(jbyte) * width * height * CHANNELS);
    env->ReleaseByteArrayElements(data, yuvData, JNI_ABORT);

    if (width > 0 && height > 0) {
        cxx::CameraHelper::get()->newTexture(newData, width, height, CHANNELS);
    }
}
