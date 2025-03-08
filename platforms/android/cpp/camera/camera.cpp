#include "camera.h"

#include <android/log.h>
#include <opencv4/opencv2/opencv.hpp>

cxx::Texture::Texture(jbyte * data, int width, int height, int channels)
  : data{ data }
  , width{ width }
  , height{ height }
  , channels{ channels } {
}

cxx::Texture::~Texture() = default;

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
