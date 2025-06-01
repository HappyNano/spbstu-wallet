#include "camera.h"

#include <spdlog/spdlog.h>

using namespace cxx;

AndroidCamera::AndroidCamera(
 std::weak_ptr< AndroidMainActivity > mainActivity,
 std::shared_ptr< CameraSink > cameraSink)
  : ICamera(std::move(cameraSink))
  , mainActivity_(std::move(mainActivity)) {
}

void AndroidCamera::openCamera() {
    if (auto ma = mainActivity_.lock()) {
        ma->openCamera();
    } else {
        SPDLOG_ERROR("AndroidCamera: openCamera: expired mainActivity");
    }
}

void AndroidCamera::closeCamera() {
    if (auto ma = mainActivity_.lock()) {
        ma->closeCamera();
    } else {
        SPDLOG_ERROR("AndroidCamera: closeCamera: expired mainActivity");
    }
}
