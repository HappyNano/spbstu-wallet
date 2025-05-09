#include "i_camera.h"

using namespace cxx;

ICamera::ICamera(std::shared_ptr< CameraSink > cameraSink)
  : cameraSink_(std::move(cameraSink)) {
}

std::shared_ptr< Frame > ICamera::lastFrame() {
    return cameraSink_->getLastFrame();
}
