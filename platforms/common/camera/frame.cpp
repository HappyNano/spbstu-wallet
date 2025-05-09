#include "frame.h"

#include <opencv4/opencv2/core.hpp>

using namespace cxx;

void Frame::rotate() {
    cv::Mat mat(height, width, CV_MAKETYPE(CV_8U, channels), data.get());
    width = 640;
    height = 480;
    memcpy(
     data.get(),
     mat(cv::Rect((mat.cols - width) / 2, (mat.rows - height) / 2, width, height)).clone().data,
     width * height * channels);

    cv::Mat rgbaMat(height, width, CV_MAKETYPE(CV_8U, channels), data.get());
    cv::Mat rotatedMat;
    cv::rotate(rgbaMat, rotatedMat, cv::ROTATE_90_CLOCKWISE);
    memcpy(data.get(), rotatedMat.data, width * height * channels);
    std::swap(width, height);
}

auto Frame::isVertical() const -> bool {
    return width < height;
}
