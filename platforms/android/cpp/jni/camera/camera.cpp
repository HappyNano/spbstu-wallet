#include "camera.h"

#include <android/log.h>

#include <GLES3/gl3.h>
#include <imgui.h>
#include <mutex>

jni::Texture::Texture(jbyte * data, int width, int height)
  : data{ data }
  , width{ width }
  , height{ height } {
}

jni::Texture::~Texture() {
    if (data) {
        delete[] data;
    }
}

auto jni::CameraHelper::addTexture(jbyte * data, int width, int height) -> void {
    std::lock_guard< std::mutex > lock(mutex_);
    lastTexture_ = std::make_shared< Texture >(data, width, height);
    __android_log_print(ANDROID_LOG_INFO, "CameraHelperJni", "New data is %u width %i height %i addr %llu", 0, width, height, (unsigned long long)lastTexture_.get());
}

auto jni::CameraHelper::last() const -> Texture::shared {
    std::lock_guard< std::mutex > lock(mutex_);
    return lastTexture_;
}

extern "C" JNIEXPORT void JNICALL
 Java_imgui_example_android_CameraHelper_onImageAvailable(JNIEnv * env, jclass, jbyteArray data, jint width, jint height) {
    __android_log_print(ANDROID_LOG_INFO, "CameraHelperJni", "%s", "New data avaible");

    // glEnable(GL_TEXTURE_2D);
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    jbyte * yuvData = env->GetByteArrayElements(data, nullptr);

    auto * newData = new jbyte[width * height * 4];
    memcpy(newData, yuvData, sizeof(jbyte) * width * height * 4);
    env->ReleaseByteArrayElements(data, yuvData, JNI_ABORT);

    // Отображение в ImGui
    jni::CameraHelper::get()->addTexture(newData, width, height);
}
