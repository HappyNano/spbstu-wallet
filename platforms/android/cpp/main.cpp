#include <spdlog/spdlog.h>

#include <platforms/android/cpp/camera/camera.h>
#include <platforms/android/cpp/camera/jni/sink/jni_camera_sink.h>
#include <platforms/android/cpp/main_activity/android_main_activity.h>

namespace {
    constexpr auto BACKGROUD_COLOR = ImVec4(217 / 255.f, 219 / 255.f, 218 / 255.f, 1.0f);
} // unnamed namespace

extern "C"
{
    // NOLINTNEXTLINE(readability-identifier-naming)
    void android_main(struct android_app * app) {
        SPDLOG_INFO("android_main: %s", "start");

        auto mainActivity = std::make_shared< cxx::AndroidMainActivity >(app);
        cxx::AndroidMainActivity::set(mainActivity);

        auto camera = std::make_shared< cxx::AndroidCamera >(
         mainActivity,
         cxx::JniCameraSink::get());

        auto mainLoop = std::make_unique< cxx::MainLoop >(
         camera);
        mainActivity->setMainLoop(std::move(mainLoop));

        mainActivity->setBackgroudColor(BACKGROUD_COLOR);

        mainActivity->run();
    }
}
