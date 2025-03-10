#pragma once

#include <jni.h>

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <imgui.h>

#include <platforms/android/cpp/main_activity/jni_executor.h>
#include <utils/singleton/singleton.h>

#include <filesystem>
#include <optional>
#include <string_view>

namespace cxx {
    class MainActivity final: public util::Singleton< MainActivity > {
    public:
        static constexpr const char * INI_FILENAME = "imgui.ini";

    public:
        void init(struct android_app * app);
        void shutdown();

        void mainLoopStep(const std::function< void(void) > & drawFunction);

    public:
        ~MainActivity() override;

        auto isInitialized() const noexcept -> bool;
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
        void closeCameraUnsafe();
        void openCameraUnsafe();
        void showSoftKeyboardInputUnsafe();
        void hideSoftKeyboardInputUnsafe();
        void pollUnicodeCharsUnsafe();
        void getStatusBarHeightUnsafe();

        auto createExecutorUnsafe(std::string_view name, std::string_view sign) -> JNIExecutor;

    private:
        bool initialized_ = false;

        EGLDisplay eglDisplay_ = EGL_NO_DISPLAY;
        EGLSurface eglSurface_ = EGL_NO_SURFACE;
        EGLContext eglContext_ = EGL_NO_CONTEXT;

        struct android_app * app_ = nullptr;

        std::filesystem::path iniPath_ = "";
        bool wantTextInputLast_ = false;

        std::optional< int > statusBarHeight_;
        ImVec4 backgroudColor_ = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    };
} // namespace cxx
