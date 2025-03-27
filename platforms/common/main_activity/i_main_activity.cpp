#include "i_main_activity.h"

using namespace cxx;

IMainActivity::IMainActivity()
  : Context{
      .statusBarHeight = 0,
      .backgroudColor = ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f }
} {}

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

void IMainActivity::mainLoopStep() {
    mainLoopStepImpl();
}

auto IMainActivity::isInitialized() const noexcept -> bool {
    return initialized_;
}

void IMainActivity::setMainLoop(std::unique_ptr< MainLoop > mainLoop) {
    mainLoop_ = std::move(mainLoop);
}
