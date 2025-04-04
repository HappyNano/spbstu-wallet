#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>

#include <spdlog/spdlog.h>

#include <platforms/common/camera/null_camera.h>
#include <platforms/sdl/main_activity/sdl_main_activity.h>

namespace {
    constexpr auto BACKGROUD_COLOR = ImVec4(217 / 255.f, 219 / 255.f, 218 / 255.f, 1.0f);
} // unnamed namespace

int main(int, char **) {
    SPDLOG_INFO("android_main: %s", "start");

    auto mainActivity = std::make_shared< cxx::SDLMainActivity >();
    cxx::SDLMainActivity::set(mainActivity);

    auto mainLoop = std::make_unique< cxx::MainLoop >(
     std::make_shared< cxx::NullCamera >());
    mainActivity->setMainLoop(std::move(mainLoop));

    mainActivity->setBackgroudColor(BACKGROUD_COLOR);

    mainActivity->run();
    return 0;
}
