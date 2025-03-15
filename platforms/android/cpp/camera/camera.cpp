#include "camera.h"

#include <android/log.h>
#include <memory>
#include <opencv4/opencv2/opencv.hpp>

cxx::Texture::Texture(jbyte * data, int width, int height, int channels)
  : data{ data }
  , width{ width }
  , height{ height }
  , channels{ channels } {
}

cxx::Texture::Texture(const Texture & obj)
  : data{ std::make_unique< jbyte >(obj.width * obj.height * obj.channels) }
  , width{ obj.width }
  , height{ obj.height }
  , channels{ obj.channels } {
    memcpy(data.get(), obj.data.get(), width * height * channels);
}

cxx::Texture::Texture(Texture && obj)
  : data{ std::move(obj.data) }
  , width{ obj.width }
  , height{ obj.height }
  , channels{ obj.channels } {
    obj.data.release();
    obj.width = 0;
    obj.height = 0;
    obj.channels = 0;
}

cxx::Texture::~Texture() = default;

auto cxx::Texture::operator=(const Texture & obj) -> Texture & {
    if (this != std::addressof(obj)) {
        Texture tmp(obj);
        swap(tmp);
    }
    return *this;
}

auto cxx::Texture::operator=(Texture && obj) -> Texture & {
    if (this != std::addressof(obj)) {
        Texture tmp(std::move(obj));
        swap(tmp);
    }
    return *this;
}

void cxx::Texture::swap(Texture & obj) noexcept {
    data.swap(obj.data);
    std::swap(width, obj.width);
    std::swap(height, obj.height);
    std::swap(channels, obj.channels);
}

auto cxx::Texture::rotate() -> void {
    cv::Mat rgbaMat(height, width, CV_MAKETYPE(CV_8U, channels), data.get());
    cv::Mat rotatedMat;
    cv::rotate(rgbaMat, rotatedMat, cv::ROTATE_90_CLOCKWISE);
    memcpy(data.get(), rotatedMat.data, width * height * channels);
    std::swap(width, height);
}

auto cxx::Texture::isVertical() const -> bool {
    return width < height;
}

auto cxx::CameraHelper::newTexture(jbyte * data, int width, int height, int channels) -> void {
    std::lock_guard< std::mutex > lock(mutex_);
    lastTexture_ = std::make_shared< Texture >(data, width, height, channels);
    __android_log_print(ANDROID_LOG_INFO, "CameraHelperJni", "New data width %i height %i addr %llu", width, height, (unsigned long long)lastTexture_.get());
}

auto cxx::CameraHelper::last() const -> Texture::shared {
    std::lock_guard< std::mutex > lock(mutex_);
    return lastTexture_;
}
