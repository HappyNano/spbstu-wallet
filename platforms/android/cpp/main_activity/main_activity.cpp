#include "main_activity.h"

#include <imgui.h>
#include <imgui_impl_android.h>
#include <imgui_impl_opengl3.h>

#include <platforms/android/cpp/logger/logger.h>

#include <exception>
#include <filesystem>

namespace {

    template < typename Class, typename Func >
    int safeCallMethod(Class * cls, Func func, const char * fmt) {
        try {
            (cls->*func)();
        } catch (const std::exception & e) {
            cxx::AndroidLogger::logError(fmt, e.what());
            return -1;
        }

        return 0;
    }

} // unnamed namespace

cxx::MainActivity::~MainActivity() {
    shutdown();
}

void cxx::MainActivity::init(struct android_app * app) {
    cxx::AndroidLogger::logInfo("MainActivity: %s", "Initialization");
    if (initialized_) {
        return;
    }

    app_ = app;
    ANativeWindow_acquire(app_->window);

    // Initialize EGL
    // This is mostly boilerplate code for EGL...
    {
        eglDisplay_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (eglDisplay_ == EGL_NO_DISPLAY) {
            AndroidLogger::logError("%s", "eglGetDisplay(EGL_DEFAULT_DISPLAY) returned EGL_NO_DISPLAY");
        }

        if (eglInitialize(eglDisplay_, nullptr, nullptr) != EGL_TRUE) {
            AndroidLogger::logError("%s", "eglInitialize() returned with an error");
        }

        const EGLint eglAttributes[] = { EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_DEPTH_SIZE, 24, EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE };
        EGLint numConfigs = 0;
        if (eglChooseConfig(eglDisplay_, eglAttributes, nullptr, 0, &numConfigs) != EGL_TRUE) {
            AndroidLogger::logError("%s", "eglChooseConfig() returned with an error");
        }
        if (numConfigs == 0) {
            AndroidLogger::logError("%s", "eglChooseConfig() returned 0 matching config");
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
            AndroidLogger::logError("%s", "eglCreateContext() returned EGL_NO_CONTEXT");
        }

        eglSurface_ = eglCreateWindowSurface(eglDisplay_, eglConfig, app_->window, nullptr);
        eglMakeCurrent(eglDisplay_, eglSurface_, eglSurface_, eglContext_);
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO & io = ImGui::GetIO();

    iniPath_ = std::filesystem::path(app->activity->internalDataPath) / INI_FILENAME;
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

    cxx::AndroidLogger::logInfo("MainActivity: %s", "Successful initialized");
    initialized_ = true;
}

void cxx::MainActivity::shutdown() {
    cxx::AndroidLogger::logInfo("MainActivity: %s", "Shutdown");
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

    initialized_ = false;
}

auto cxx::MainActivity::isInitialized() const noexcept -> bool {
    return initialized_;
}
auto cxx::MainActivity::getEGLDisplay() -> const EGLDisplay & {
    return eglDisplay_;
}
auto cxx::MainActivity::getEGLSurface() -> const EGLSurface & {
    return eglSurface_;
}
auto cxx::MainActivity::getEGLContext() -> const EGLContext & {
    return eglContext_;
}

void cxx::MainActivity::setBackgroudColor(ImVec4 color) {
    backgroudColor_ = color;
}

void cxx::MainActivity::mainLoopStep(const std::function< void(void) > & drawFunction) {
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

    drawFunction();

    // Rendering
    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(backgroudColor_.x, backgroudColor_.y, backgroudColor_.z, backgroudColor_.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    eglSwapBuffers(getEGLDisplay(), getEGLSurface());
}

auto cxx::MainActivity::closeCamera() noexcept -> int {
    return safeCallMethod(this, &MainActivity::closeCameraUnsafe, "closeCameraUnsafe: %s");
}

auto cxx::MainActivity::openCamera() noexcept -> int {
    return safeCallMethod(this, &MainActivity::openCameraUnsafe, "openCameraUnsafe: %s");
}

auto cxx::MainActivity::showSoftKeyboardInput() noexcept -> int {
    return safeCallMethod(this, &MainActivity::showSoftKeyboardInputUnsafe, "showSoftKeyboardInput: %s");
}

auto cxx::MainActivity::hideSoftKeyboardInput() noexcept -> int {
    return safeCallMethod(this, &MainActivity::hideSoftKeyboardInputUnsafe, "hideSoftKeyboardInput: %s");
}

auto cxx::MainActivity::pollUnicodeChars() noexcept -> int {
    return safeCallMethod(this, &MainActivity::pollUnicodeCharsUnsafe, "pollUnicodeChars: %s");
}

auto cxx::MainActivity::getStatusBarHeight() noexcept -> int {
    if (!statusBarHeight_.has_value()) {
        getStatusBarHeightUnsafe();
    }
    return statusBarHeight_.value_or(0);
}

void cxx::MainActivity::closeCameraUnsafe() {
    auto executor = createExecutorUnsafe("closeCamera", "()V");
    executor.call< void >();
}

void cxx::MainActivity::openCameraUnsafe() {
    auto executor = createExecutorUnsafe("openCamera", "()V");
    // jstring message = executor.getJavaEnv()->NewStringUTF("Привет из C++!!!!");
    executor.call< void >();
}

void cxx::MainActivity::showSoftKeyboardInputUnsafe() {
    auto executor = createExecutorUnsafe("showSoftInput", "()V");
    executor.call< void >();
}

void cxx::MainActivity::hideSoftKeyboardInputUnsafe() {
    auto executor = createExecutorUnsafe("hideSoftInput", "()V");
    executor.call< void >();
}

// Unfortunately, the native KeyEvent implementation has no getUnicodeChar() function.
// Therefore, we implement the processing of KeyEvents in MainActivity.kt and poll
// the resulting Unicode characters here via JNI and send them to Dear ImGui.
void cxx::MainActivity::pollUnicodeCharsUnsafe() {
    auto executor = createExecutorUnsafe("pollUnicodeChar", "()I");

    // Send the actual characters to Dear ImGui
    ImGuiIO & io = ImGui::GetIO();
    jint unicodeCharacter;
    while ((unicodeCharacter = executor.call< jint >()) != 0) {
        io.AddInputCharacter(unicodeCharacter);
    }
}

void cxx::MainActivity::getStatusBarHeightUnsafe() {
    auto executor = createExecutorUnsafe("getStatusBarHeight", "()I");
    statusBarHeight_.emplace(executor.call< jint >());
}

auto cxx::MainActivity::createExecutorUnsafe(std::string_view name, std::string_view sign) -> JNIExecutor {
    return JNIExecutor(
     app_->activity->vm,
     app_->activity->clazz,
     std::move(name),
     std::move(sign));
}
