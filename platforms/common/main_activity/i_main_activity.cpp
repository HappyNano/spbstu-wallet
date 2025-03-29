#include "i_main_activity.h"

using namespace cxx;

IMainActivity::IMainActivity() = default;

void IMainActivity::init() {
    if (!initialized_) {
        initImpl();
    }
}

void IMainActivity::shutdown() {
    if (initialized_) {
        shutdownImpl();
    }
}

auto IMainActivity::isInitialized() const noexcept -> bool {
    return initialized_;
}

void IMainActivity::setMainLoop(std::unique_ptr< MainLoop > mainLoop) {
    mainLoop_ = std::move(mainLoop);
}
