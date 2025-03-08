// dear imgui: standalone example application for Android + OpenGL ES 3

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/asset_manager.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <imgui.h>
#include <imgui_impl_android.h>
#include <imgui_impl_opengl3.h>
#include <string>

#include <opencv4/opencv2/opencv.hpp>
#include <platforms/android/cpp/camera/camera.h>

// Data
static EGLDisplay gEglDisplay    = EGL_NO_DISPLAY;
static EGLSurface gEglSurface    = EGL_NO_SURFACE;
static EGLContext gEglContext    = EGL_NO_CONTEXT;
static struct android_app * gApp = nullptr;
static bool gInitialized         = false;
static char gLogTag[]            = "AndroidCppLib";
static std::string gIniFilename  = "";

// Forward declarations of helper functions
static void init(struct android_app * app);
static void shutdown();
static void mainLoopStep();
static int showSoftKeyboardInput();
static int pollUnicodeChars();

// Main code
static void handleAppCmd(struct android_app * app, int32_t appCmd) {
    switch (appCmd)
    {
    case APP_CMD_SAVE_STATE:
        break;
    case APP_CMD_INIT_WINDOW:
        init(app);
        break;
    case APP_CMD_TERM_WINDOW:
        shutdown();
        break;
    case APP_CMD_GAINED_FOCUS:
    case APP_CMD_LOST_FOCUS:
        break;
    }
}

static int32_t handleInputEvent(struct android_app * /*app*/, AInputEvent * inputEvent) {
    return ImGui_ImplAndroid_HandleInputEvent(inputEvent);
}

void android_main(struct android_app * app) { // NOLINT(readability-identifier-naming)
    // Make sure glue isn't stripped.

    app->onAppCmd     = handleAppCmd;
    app->onInputEvent = handleInputEvent;

    while (true)
    {
        int outEvents;
        struct android_poll_source * outData;

        // Poll all events. If the app is not visible, this loop blocks until gInitialized == true.
        while (ALooper_pollOnce(gInitialized ? 0 : -1, nullptr, &outEvents, (void **)&outData) >= 0)
        {
            // Process one event
            if (outData != nullptr) {
                outData->process(app, outData);
            }

            // Exit the app by returning from within the infinite loop
            if (app->destroyRequested != 0)
            {
                // shutdown() should have been called already while processing the
                // app command APP_CMD_TERM_WINDOW. But we play save here
                if (!gInitialized) {
                    shutdown();
                }

                return;
            }
        }

        // Initiate a new frame
        mainLoopStep();
    }
}

void init(struct android_app * app) {
    if (gInitialized) {
        return;
    }

    gApp = app;
    ANativeWindow_acquire(gApp->window);

    // Initialize EGL
    // This is mostly boilerplate code for EGL...
    {
        gEglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (gEglDisplay == EGL_NO_DISPLAY) {
            __android_log_print(ANDROID_LOG_ERROR, gLogTag, "%s", "eglGetDisplay(EGL_DEFAULT_DISPLAY) returned EGL_NO_DISPLAY");
        }

        if (eglInitialize(gEglDisplay, nullptr, nullptr) != EGL_TRUE) {
            __android_log_print(ANDROID_LOG_ERROR, gLogTag, "%s", "eglInitialize() returned with an error");
        }

        const EGLint eglAttributes[] = { EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_DEPTH_SIZE, 24, EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE };
        EGLint numConfigs            = 0;
        if (eglChooseConfig(gEglDisplay, eglAttributes, nullptr, 0, &numConfigs) != EGL_TRUE) {
            __android_log_print(ANDROID_LOG_ERROR, gLogTag, "%s", "eglChooseConfig() returned with an error");
        }
        if (numConfigs == 0) {
            __android_log_print(ANDROID_LOG_ERROR, gLogTag, "%s", "eglChooseConfig() returned 0 matching config");
        }

        // Get the first matching config
        EGLConfig eglConfig;
        eglChooseConfig(gEglDisplay, eglAttributes, &eglConfig, 1, &numConfigs);
        EGLint eglFormat;
        eglGetConfigAttrib(gEglDisplay, eglConfig, EGL_NATIVE_VISUAL_ID, &eglFormat);
        ANativeWindow_setBuffersGeometry(gApp->window, 0, 0, eglFormat);

        const EGLint eglContextAttributes[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
        gEglContext                         = eglCreateContext(gEglDisplay, eglConfig, EGL_NO_CONTEXT, eglContextAttributes);

        if (gEglContext == EGL_NO_CONTEXT) {
            __android_log_print(ANDROID_LOG_ERROR, gLogTag, "%s", "eglCreateContext() returned EGL_NO_CONTEXT");
        }

        gEglSurface = eglCreateWindowSurface(gEglDisplay, eglConfig, gApp->window, nullptr);
        eglMakeCurrent(gEglDisplay, gEglSurface, gEglSurface, gEglContext);
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO & io = ImGui::GetIO();

    // Redirect loading/saving of .ini file to our location.
    // Make sure 'gIniFilename' persists while we use Dear ImGui.
    gIniFilename   = std::string(app->activity->internalDataPath) + "/imgui.ini";
    io.IniFilename = gIniFilename.c_str();
    ;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplAndroid_Init(gApp->window);
    ImGui_ImplOpenGL3_Init("#version 300 es");

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Android: The TTF files have to be placed into the assets/ directory (android/app/src/main/assets), we use our GetAssetData() helper to retrieve them.

    // We load the default font with increased size to improve readability on many devices with "high" DPI.
    // FIXME: Put some effort into DPI awareness.
    // Important: when calling AddFontFromMemoryTTF(), ownership of font_data is transferred by Dear ImGui by default (deleted is handled by Dear ImGui), unless we set FontDataOwnedByAtlas=false in ImFontConfig
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
    // FIXME: Put some effort into DPI awareness
    ImGui::GetStyle().ScaleAllSizes(3.0f);

    gInitialized = true;
}

void mainLoopStep() {
    ImGuiIO & io = ImGui::GetIO();
    if (gEglDisplay == EGL_NO_DISPLAY) {
        return;
    }

    // Our state
    // (we use static, which essentially makes the variable globals, as a convenience to keep the example code easy to follow)
    static bool showDemoWindow    = true;
    static bool showAnotherWindow = false;
    static bool showCamera        = true;
    static ImVec4 clearColor      = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Poll Unicode characters via JNI
    // FIXME: do not call this every frame because of JNI overhead
    pollUnicodeChars();

    // Open on-screen (soft) input if requested by Dear ImGui
    static bool wantTextInputLast = false;
    if (io.WantTextInput && !wantTextInputLast) {
        showSoftKeyboardInput();
    }
    wantTextInputLast = io.WantTextInput;

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame();
    ImGui::NewFrame();

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        static float f     = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");        // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &showDemoWindow); // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &showAnotherWindow);
        ImGui::Checkbox("Camera", &showCamera);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float *)&clearColor); // Edit 3 floats representing a color

        if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
        {
            counter++;
        }
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    if (showAnotherWindow)
    {
        ImGui::Begin("Another Window", &showAnotherWindow); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me")) {
            showAnotherWindow = false;
        }
        ImGui::End();
    }

    static GLuint textureId = 0;
    if (!textureId) {
        // Генерация текстуры
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        if (!glIsTexture(textureId)) {
            __android_log_print(ANDROID_LOG_ERROR, "MainCppCameraHelper", "Failed to generate OpenGL texture!");
        }
    }

    if (showCamera) {
        ImGui::Begin("Camera Window", &showCamera);
        ImGui::Text("Hello from another window!");
        static bool rotateImg = true;
        ImGui::Checkbox("Rotate", &rotateImg);
        if (auto lastTexture = cxx::CameraHelper::get()->last(); lastTexture && textureId) {
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBindTexture(GL_TEXTURE_2D, textureId);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            if (rotateImg && !lastTexture->isVertical()) {
                lastTexture->rotate();
            }
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, lastTexture->width, lastTexture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, lastTexture->data.get());

            ImGui::Image(textureId, ImVec2(lastTexture->width, lastTexture->height));
            ImGui::Text("pointer = %x", textureId);
            ImGui::Text("size = %d x %d", lastTexture->width, lastTexture->height);
        }
        if (ImGui::Button("Close Me")) {
            showCamera = false;
        }
        ImGui::End();
    }

    // Rendering
    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    if (currentProgram == 0) {
        __android_log_print(ANDROID_LOG_ERROR, "MainCppCameraHelper", "No OpenGL context! HERE");
    }
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    eglSwapBuffers(gEglDisplay, gEglSurface);
    // if (texture) {
    //     __android_log_print(ANDROID_LOG_INFO, "MainCppCameraHelper", "Deleted %u", texture);
    //     glDeleteTextures(1, &texture);
    // }
}

void shutdown() {
    if (!gInitialized) {
        return;
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();

    if (gEglDisplay != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(gEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (gEglContext != EGL_NO_CONTEXT) {
            eglDestroyContext(gEglDisplay, gEglContext);
        }

        if (gEglSurface != EGL_NO_SURFACE) {
            eglDestroySurface(gEglDisplay, gEglSurface);
        }

        eglTerminate(gEglDisplay);
    }

    gEglDisplay = EGL_NO_DISPLAY;
    gEglContext = EGL_NO_CONTEXT;
    gEglSurface = EGL_NO_SURFACE;
    ANativeWindow_release(gApp->window);

    gInitialized = false;
}

// Helper functions

// Unfortunately, there is no way to show the on-screen input from native code.
// Therefore, we call ShowSoftKeyboardInput() of the main activity implemented in MainActivity.kt via JNI.
static int showSoftKeyboardInput() {
    JavaVM * javaVm  = gApp->activity->vm;
    JNIEnv * javaEnv = nullptr;

    jint jniReturn = javaVm->GetEnv((void **)&javaEnv, JNI_VERSION_1_6);
    if (jniReturn == JNI_ERR) {
        return -1;
    }

    jniReturn = javaVm->AttachCurrentThread(&javaEnv, nullptr);
    if (jniReturn != JNI_OK) {
        return -2;
    }

    jclass nativeActivityClazz = javaEnv->GetObjectClass(gApp->activity->clazz);
    if (nativeActivityClazz == nullptr) {
        return -3;
    }

    jmethodID methodId = javaEnv->GetMethodID(nativeActivityClazz, "showSoftInput", "()V");
    if (methodId == nullptr) {
        return -4;
    }

    javaEnv->CallVoidMethod(gApp->activity->clazz, methodId);

    jniReturn = javaVm->DetachCurrentThread();
    if (jniReturn != JNI_OK) {
        return -5;
    }

    return 0;
}

// Unfortunately, the native KeyEvent implementation has no getUnicodeChar() function.
// Therefore, we implement the processing of KeyEvents in MainActivity.kt and poll
// the resulting Unicode characters here via JNI and send them to Dear ImGui.
static int pollUnicodeChars() {
    JavaVM * javaVm  = gApp->activity->vm;
    JNIEnv * javaEnv = nullptr;

    jint jniReturn = javaVm->GetEnv((void **)&javaEnv, JNI_VERSION_1_6);
    if (jniReturn == JNI_ERR) {
        return -1;
    }

    jniReturn = javaVm->AttachCurrentThread(&javaEnv, nullptr);
    if (jniReturn != JNI_OK) {
        return -2;
    }

    jclass nativeActivityClazz = javaEnv->GetObjectClass(gApp->activity->clazz);
    if (nativeActivityClazz == nullptr) {
        return -3;
    }

    jmethodID methodId = javaEnv->GetMethodID(nativeActivityClazz, "pollUnicodeChar", "()I");
    if (methodId == nullptr) {
        return -4;
    }

    // Send the actual characters to Dear ImGui
    ImGuiIO & io = ImGui::GetIO();
    jint unicodeCharacter;
    while ((unicodeCharacter = javaEnv->CallIntMethod(gApp->activity->clazz, methodId)) != 0) {
        io.AddInputCharacter(unicodeCharacter);
    }

    jniReturn = javaVm->DetachCurrentThread();
    if (jniReturn != JNI_OK) {
        return -5;
    }

    return 0;
}
