#pragma once

#include <jni.h>

#include <GLES3/gl3.h>
#include <utils/singleton/singleton.h>

#include <opencv4/opencv2/opencv.hpp>

#include <mutex>

namespace jni {

    class Texture final {
    public:
        using shared = std::shared_ptr< Texture >;

        Texture(jbyte * data, int width, int height);
        ~Texture();

        jbyte * data;
        int width;
        int height;
    };

    class CameraHelper final: public util::Singleton< CameraHelper > {
    public:
        CameraHelper()           = default;
        ~CameraHelper() override = default;

        auto addTexture(jbyte * data, int width, int height) -> void;
        auto last() const -> Texture::shared;

    private:
        Texture::shared lastTexture_;

        mutable std::mutex mutex_;
    };

} // namespace jni

// Функция для обработки изображения из Java
extern "C" JNIEXPORT void JNICALL
 Java_imgui_example_android_CameraHelper_onImageAvailable(JNIEnv * env, jclass, jbyteArray data, jint width, jint height); // NOLINT(readability-identifier-naming)
