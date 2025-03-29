#pragma once

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <imgui.h>

#include <platforms/common/main_loop/main_loop.h>
#include <utils/singleton/singleton.h>

#include <filesystem>
#include <memory>

namespace cxx {
    class IMainActivity: public util::Singleton< IMainActivity >, public Context {
    public:
        static constexpr const char * INI_FILENAME = "imgui.ini";

    public:
        /**
         * @brief Initialization function
         */
        void init();
        /**
         * @brief Shutdown function
         */
        void shutdown();
        /**
         * @brief Main cycle function
         */
        virtual void run() = 0;

        ~IMainActivity() override = default;

    public:
        auto isInitialized() const noexcept -> bool;
        void setMainLoop(std::unique_ptr< MainLoop > mainLoop);

        void setBackgroudColor(ImVec4 color);

        // Helpers
        auto closeCamera() noexcept -> int;
        auto openCamera() noexcept -> int;
        auto showSoftKeyboardInput() noexcept -> int;
        auto hideSoftKeyboardInput() noexcept -> int;
        auto pollUnicodeChars() noexcept -> int;
        auto getStatusBarHeight() noexcept -> int;

    protected:
        IMainActivity();

        void closeCameraUnsafe();
        void openCameraUnsafe();
        void showSoftKeyboardInputUnsafe();
        void hideSoftKeyboardInputUnsafe();
        void pollUnicodeCharsUnsafe();
        void getStatusBarHeightUnsafe();

        /**
         * @brief Implementation of initialization function
         */
        virtual void initImpl() = 0;
        /**
         * @brief Implementation of shutdown function
         */
        virtual void shutdownImpl() = 0;

    protected:
        bool initialized_ = false;

        std::filesystem::path iniPath_ = "";
        bool wantTextInputLast_ = false;

        std::unique_ptr< MainLoop > mainLoop_;
    };
} // namespace cxx
