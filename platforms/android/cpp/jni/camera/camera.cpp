#include "camera.h"

#include <android/log.h>

#include <GLES3/gl3.h>
#include <imgui.h>

jni::Texture::Texture(jbyte * data, int width, int height, int channels)
  : data{ data }
  , width{ width }
  , height{ height }
  , channels{ channels } {
}

jni::Texture::~Texture() = default;

auto jni::Texture::rotate() -> void {
    cv::Mat rgbaMat(height, width, CV_MAKETYPE(CV_8U, channels), data.get());
    cv::Mat rotatedMat;
    cv::rotate(rgbaMat, rotatedMat, cv::ROTATE_90_CLOCKWISE);
    memcpy(data.get(), rotatedMat.data, width * height * channels);
    std::swap(width, height);
}

auto jni::Texture::isVertical() const -> bool {
    return width < height;
}

auto jni::CameraHelper::newTexture(jbyte * data, int width, int height, int channels) -> void {
    std::lock_guard< std::mutex > lock(mutex_);
    lastTexture_ = std::make_shared< Texture >(data, width, height, channels);
    __android_log_print(ANDROID_LOG_INFO, "CameraHelperJni", "New data width %i height %i addr %llu", width, height, (unsigned long long)lastTexture_.get());
}

auto jni::CameraHelper::last() const -> Texture::shared {
    std::lock_guard< std::mutex > lock(mutex_);
    return lastTexture_;
}

extern "C" JNIEXPORT void JNICALL
 Java_imgui_example_android_CameraHelper_onImageAvailable(JNIEnv * env, jclass, jbyteArray data, jint width, jint height) {
    static constexpr int CHANNELS = 4;

    __android_log_print(ANDROID_LOG_INFO, "CameraHelperJni", "%s", "New data avaible");

    jbyte * yuvData = env->GetByteArrayElements(data, nullptr);

    auto * newData = new jbyte[width * height * CHANNELS];
    memcpy(newData, yuvData, sizeof(jbyte) * width * height * CHANNELS);
    env->ReleaseByteArrayElements(data, yuvData, JNI_ABORT);

    jni::CameraHelper::get()->newTexture(newData, width, height, CHANNELS);
}
