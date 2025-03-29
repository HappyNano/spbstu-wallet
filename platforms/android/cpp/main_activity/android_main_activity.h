#pragma once

#include <jni.h>

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <imgui.h>
#include <spdlog/fwd.h>

#include <platforms/android/cpp/main_activity/jni_executor.h>
#include <platforms/common/main_activity/i_main_activity.h>
#include <utils/singleton/singleton.h>

#include <filesystem>
#include <memory>
#include <string_view>

// NOLINTNEXTLINE(readability-identifier-naming)
struct android_app;

namespace cxx {
    class AndroidMainActivity final: public IMainActivity, public std::enable_shared_from_this< AndroidMainActivity > {
    public:
        AndroidMainActivity(struct android_app * app);
        ~AndroidMainActivity() override;

        void mainLoopStep(); // override;
        // IMainActivity
        void run() override;

    public:
        auto getEGLDisplay() -> const EGLDisplay &;
        auto getEGLSurface() -> const EGLSurface &;
        auto getEGLContext() -> const EGLContext &;

        void setBackgroudColor(ImVec4 color);

        // Helpers
        auto closeCamera() noexcept -> int;
        auto openCamera() noexcept -> int;
        auto showSoftKeyboardInput() noexcept -> int;
        auto hideSoftKeyboardInput() noexcept -> int;
        auto pollUnicodeChars() noexcept -> int;
        auto getStatusBarHeight() noexcept -> int;

    private:
        // IMainActivity
        void initImpl() override;
        void shutdownImpl() override;

        void closeCameraUnsafe();
        void openCameraUnsafe();
        void showSoftKeyboardInputUnsafe();
        void hideSoftKeyboardInputUnsafe();
        void pollUnicodeCharsUnsafe();
        void getStatusBarHeightUnsafe();

        auto createExecutorUnsafe(std::string_view name, std::string_view sign) -> JNIExecutor;

    private:
        EGLDisplay eglDisplay_ = EGL_NO_DISPLAY;
        EGLSurface eglSurface_ = EGL_NO_SURFACE;
        EGLContext eglContext_ = EGL_NO_CONTEXT;

        struct android_app * app_ = nullptr;

        std::filesystem::path iniPath_ = "";
        bool wantTextInputLast_ = false;
    };
} // namespace cxx
