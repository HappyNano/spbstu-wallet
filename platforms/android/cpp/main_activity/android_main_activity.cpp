#include "android_main_activity.h"

#include <GLES3/gl3.h>
#include <android_native_app_glue.h>
#include <fmt/base.h>
#include <imgui.h>
#include <imgui_impl_android.h>
#include <imgui_impl_opengl3.h>

#include <spdlog/common.h>
#include <spdlog/sinks/android_sink.h>
#include <spdlog/spdlog.h>

#include <exception>
#include <filesystem>

namespace {

    template < typename Class, typename Func >
    int safeCallMethod(Class * cls, Func func, std::string_view) {
        try {
            (cls->*func)();
        } catch (const std::exception & e) {
            // SPDLOG_ERROR(fmt::format(fmt::runtime(fmt), e.what()).c_str());
            return -1;
        }

        return 0;
    }

    // Android native glue parts
    void handleAppCmd(struct android_app * /*app*/, int32_t appCmd) {
        switch (appCmd)
        {
        case APP_CMD_SAVE_STATE:
            break;
        case APP_CMD_INIT_WINDOW:
            cxx::AndroidMainActivity::get()->init();
            break;
        case APP_CMD_TERM_WINDOW:
            cxx::AndroidMainActivity::get()->shutdown();
            break;
        case APP_CMD_GAINED_FOCUS:
        case APP_CMD_LOST_FOCUS:
            break;
        }
    }

    int32_t handleInputEvent(struct android_app * /*app*/, AInputEvent * inputEvent) {
        return ImGui_ImplAndroid_HandleInputEvent(inputEvent);
    }

} // unnamed namespace

cxx::AndroidMainActivity::AndroidMainActivity(struct android_app * app)
  : app_{ std::move(app) } {
    app_->onAppCmd = handleAppCmd;
    app_->onInputEvent = handleInputEvent;
}

cxx::AndroidMainActivity::~AndroidMainActivity() {
    shutdown();
}

void cxx::AndroidMainActivity::initImpl() {
    if (initialized_) {
        return;
    }

    // Init logger
    auto logger = spdlog::android_logger_mt("android");
#ifndef NDEBUG
    logger->set_level(spdlog::level::debug);
#else
    logger->set_level(spdlog::level::info);
#endif
    spdlog::set_default_logger(std::move(logger));

    ANativeWindow_acquire(app_->window);

    SPDLOG_INFO("MainActivity: Initialization");

    // Initialize EGL
    // This is mostly boilerplate code for EGL...
    {
        eglDisplay_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (eglDisplay_ == EGL_NO_DISPLAY) {
            SPDLOG_ERROR("MainActivity: eglGetDisplay(EGL_DEFAULT_DISPLAY) returned EGL_NO_DISPLAY");
        }

        if (eglInitialize(eglDisplay_, nullptr, nullptr) != EGL_TRUE) {
            SPDLOG_ERROR("MainActivity: eglInitialize() returned with an error");
        }

        const EGLint eglAttributes[] = { EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_DEPTH_SIZE, 24, EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE };
        EGLint numConfigs = 0;
        if (eglChooseConfig(eglDisplay_, eglAttributes, nullptr, 0, &numConfigs) != EGL_TRUE) {
            SPDLOG_ERROR("MainActivity: eglChooseConfig() returned with an error");
        }
        if (numConfigs == 0) {
            SPDLOG_ERROR("MainActivity: eglChooseConfig() returned 0 matching config");
        }

        // Get the first matching config
        EGLConfig eglConfig;
        eglChooseConfig(eglDisplay_, eglAttributes, &eglConfig, 1, &numConfigs);
        EGLint eglFormat;
        eglGetConfigAttrib(eglDisplay_, eglConfig, EGL_NATIVE_VISUAL_ID, &eglFormat);
        ANativeWindow_setBuffersGeometry(app_->window, 0, 0, eglFormat);

        const EGLint eglContextAttributes[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };

        eglContext_ = eglCreateContext(eglDisplay_, eglConfig, EGL_NO_CONTEXT, eglContextAttributes);

        if (eglContext_ == EGL_NO_CONTEXT) {
            SPDLOG_ERROR("MainActivity: eglCreateContext() returned EGL_NO_CONTEXT");
        }

        eglSurface_ = eglCreateWindowSurface(eglDisplay_, eglConfig, app_->window, nullptr);
        eglMakeCurrent(eglDisplay_, eglSurface_, eglSurface_, eglContext_);
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO & io = ImGui::GetIO();

    iniPath_ = std::filesystem::path(app_->activity->internalDataPath) / INI_FILENAME;
    io.IniFilename = iniPath_.c_str();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplAndroid_Init(app_->window);
    ImGui_ImplOpenGL3_Init("#version 300 es");

    ImFontConfig fontCfg;
    fontCfg.SizePixels = 22.0f;
    io.Fonts->AddFontDefault(&fontCfg);
    // void* font_data;
    // int font_data_size;
    // ImFont* font;
    // font_data_size = GetAssetData("segoeui.ttf", &font_data);
    // font = io.Fonts->AddFontFromMemoryTTF(font_data, font_data_size, 16.0f);
    // IM_ASSERT(font != nullptr);
    // font_data_size = GetAssetData("DroidSans.ttf", &font_data);
    // font = io.Fonts->AddFontFromMemoryTTF(font_data, font_data_size, 16.0f);
    // IM_ASSERT(font != nullptr);
    // font_data_size = GetAssetData("Roboto-Medium.ttf", &font_data);
    // font = io.Fonts->AddFontFromMemoryTTF(font_data, font_data_size, 16.0f);
    // IM_ASSERT(font != nullptr);
    // font_data_size = GetAssetData("Cousine-Regular.ttf", &font_data);
    // font = io.Fonts->AddFontFromMemoryTTF(font_data, font_data_size, 15.0f);
    // IM_ASSERT(font != nullptr);
    // font_data_size = GetAssetData("ArialUni.ttf", &font_data);
    // font = io.Fonts->AddFontFromMemoryTTF(font_data, font_data_size, 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    // IM_ASSERT(font != nullptr);

    // Arbitrary scale-up
    ImGui::GetStyle().ScaleAllSizes(3.0f);

    SPDLOG_INFO("MainActivity: Successful initialized");
    initialized_ = true;
}

void cxx::AndroidMainActivity::shutdownImpl() {
    SPDLOG_INFO("MainActivity: Shutdown");
    if (!initialized_) {
        return;
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();

    if (eglDisplay_ != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(eglDisplay_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (eglContext_ != EGL_NO_CONTEXT) {
            eglDestroyContext(eglDisplay_, eglContext_);
        }

        if (eglSurface_ != EGL_NO_SURFACE) {
            eglDestroySurface(eglDisplay_, eglSurface_);
        }

        eglTerminate(eglDisplay_);
    }

    eglDisplay_ = EGL_NO_DISPLAY;
    eglContext_ = EGL_NO_CONTEXT;
    eglSurface_ = EGL_NO_SURFACE;
    ANativeWindow_release(app_->window);

    // Reset logger
    spdlog::shutdown();

    initialized_ = false;
}

auto cxx::AndroidMainActivity::getEGLDisplay() -> const EGLDisplay & {
    return eglDisplay_;
}
auto cxx::AndroidMainActivity::getEGLSurface() -> const EGLSurface & {
    return eglSurface_;
}
auto cxx::AndroidMainActivity::getEGLContext() -> const EGLContext & {
    return eglContext_;
}

void cxx::AndroidMainActivity::setBackgroudColor(ImVec4 color) {
    backgroudColor_ = color;
}

auto cxx::AndroidMainActivity::closeCamera() noexcept -> int {
    return safeCallMethod(this, &AndroidMainActivity::closeCameraUnsafe, "MainActivity: closeCameraUnsafe: %s");
}

auto cxx::AndroidMainActivity::openCamera() noexcept -> int {
    return safeCallMethod(this, &AndroidMainActivity::openCameraUnsafe, "MainActivity: openCameraUnsafe: %s");
}

auto cxx::AndroidMainActivity::showSoftKeyboardInput() noexcept -> int {
    return safeCallMethod(this, &AndroidMainActivity::showSoftKeyboardInputUnsafe, "MainActivity: showSoftKeyboardInput: %s");
}

auto cxx::AndroidMainActivity::hideSoftKeyboardInput() noexcept -> int {
    return safeCallMethod(this, &AndroidMainActivity::hideSoftKeyboardInputUnsafe, "MainActivity: hideSoftKeyboardInput: %s");
}

auto cxx::AndroidMainActivity::pollUnicodeChars() noexcept -> int {
    return safeCallMethod(this, &AndroidMainActivity::pollUnicodeCharsUnsafe, "MainActivity: pollUnicodeChars: %s");
}

auto cxx::AndroidMainActivity::getStatusBarHeight() noexcept -> std::optional< int > {
    if (!statusBarHeight_.has_value()) {
        getStatusBarHeightUnsafe();
    }
    return statusBarHeight_;
}

void cxx::AndroidMainActivity::mainLoopStep() {
    ImGuiIO & io = ImGui::GetIO();
    if (getEGLDisplay() == EGL_NO_DISPLAY) {
        return;
    }

    pollUnicodeChars();

    // Open on-screen (soft) input if requested by Dear ImGui
    if (io.WantTextInput && !wantTextInputLast_) {
        showSoftKeyboardInput();
    } else if (!io.WantTextInput && wantTextInputLast_) {
        hideSoftKeyboardInput();
    }
    wantTextInputLast_ = io.WantTextInput;

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame();
    ImGui::NewFrame();

    mainLoop_->draw(shared_from_this());

    // Rendering
    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    auto backgroudColor = getBackgroundColor().value_or(ImVec4(1, 1, 1, 1));
    glClearColor(backgroudColor.x, backgroudColor.y, backgroudColor.z, backgroudColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    eglSwapBuffers(getEGLDisplay(), getEGLSurface());
}

void cxx::AndroidMainActivity::run() {
    while (true)
    {
        int outEvents;
        struct android_poll_source * outData;

        // Poll all events. If the app is not visible, this loop blocks until gInitialized == true.
        while (ALooper_pollOnce(isInitialized() ? 0 : -1, nullptr, &outEvents, (void **)&outData) >= 0)
        {
            // Process one event
            if (outData != nullptr) {
                outData->process(app_, outData);
            }

            // Exit the app by returning from within the infinite loop
            if (app_->destroyRequested != 0)
            {
                // shutdown() should have been called already while processing the
                // app command APP_CMD_TERM_WINDOW. But we play save here
                if (!isInitialized()) {
                    shutdown();
                }
                return;
            }
        }

        // Initiate a new frame
        mainLoopStep();
    }
}

void cxx::AndroidMainActivity::closeCameraUnsafe() {
    auto executor = createExecutorUnsafe("closeCamera", "()V");
    executor.call< void >();
}

void cxx::AndroidMainActivity::openCameraUnsafe() {
    auto executor = createExecutorUnsafe("openCamera", "()V");
    // jstring message = executor.getJavaEnv()->NewStringUTF("Привет из C++!!!!");
    executor.call< void >();
}

void cxx::AndroidMainActivity::showSoftKeyboardInputUnsafe() {
    auto executor = createExecutorUnsafe("showSoftInput", "()V");
    executor.call< void >();
}

void cxx::AndroidMainActivity::hideSoftKeyboardInputUnsafe() {
    auto executor = createExecutorUnsafe("hideSoftInput", "()V");
    executor.call< void >();
}

// Unfortunately, the native KeyEvent implementation has no getUnicodeChar() function.
// Therefore, we implement the processing of KeyEvents in MainActivity.kt and poll
// the resulting Unicode characters here via JNI and send them to Dear ImGui.
void cxx::AndroidMainActivity::pollUnicodeCharsUnsafe() {
    auto executor = createExecutorUnsafe("pollUnicodeChar", "()I");

    // Send the actual characters to Dear ImGui
    ImGuiIO & io = ImGui::GetIO();
    jint unicodeCharacter;
    while ((unicodeCharacter = executor.call< jint >()) != 0) {
        io.AddInputCharacter(unicodeCharacter);
    }
}

void cxx::AndroidMainActivity::getStatusBarHeightUnsafe() {
    auto executor = createExecutorUnsafe("getStatusBarHeight", "()I");
    statusBarHeight_.emplace(executor.call< jint >());
}

auto cxx::AndroidMainActivity::createExecutorUnsafe(std::string_view name, std::string_view sign) -> JNIExecutor {
    return JNIExecutor(
     app_->activity->vm,
     app_->activity->clazz,
     std::move(name),
     std::move(sign));
}
