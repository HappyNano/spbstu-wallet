#include "jni_camera.h"

#include <spdlog/spdlog.h>

#include <platforms/android/cpp/camera/jni/sink/jni_camera_sink.h>

extern "C" JNIEXPORT void JNICALL
 Java_imgui_example_android_CameraHelper_onImageAvailable(JNIEnv * env, jclass, jbyteArray data, jint width, jint height) {
    static constexpr int CHANNELS = 4;

    SPDLOG_INFO("[JNI] onImageAvailable: %s", "New data avaible");

    if (width > 0 && height > 0) {
        jbyte * imgData = env->GetByteArrayElements(data, nullptr);
        cxx::JniCameraSink::get()->loadNewFrameFromJni(imgData, width, height, CHANNELS);
        env->ReleaseByteArrayElements(data, imgData, JNI_ABORT);
    }
}
