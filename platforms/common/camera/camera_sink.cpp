#include "camera_sink.h"
#include <mutex>

using namespace cxx;

auto CameraSink::getLastFrame() const -> std::shared_ptr< Frame > {
    std::lock_guard lock(mutex_);
    return lastFrame_;
}

void CameraSink::loadNewFrame(std::shared_ptr< Frame > newFrame) {
    std::lock_guard lock(mutex_);
    lastFrame_ = std::move(newFrame);
}
