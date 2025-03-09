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

#include <opencv4/opencv2/opencv.hpp>
#include <platforms/android/cpp/camera/camera.h>
#include <platforms/android/cpp/logger/logger.h>
#include <platforms/android/cpp/main_activity/main_activity.h>

// Android native glue parts
static void handleAppCmd(struct android_app * app, int32_t appCmd) {
    switch (appCmd)
    {
    case APP_CMD_SAVE_STATE:
        break;
    case APP_CMD_INIT_WINDOW:
        cxx::MainActivity::get()->init(app);
        break;
    case APP_CMD_TERM_WINDOW:
        cxx::MainActivity::get()->shutdown();
        break;
    case APP_CMD_GAINED_FOCUS:
    case APP_CMD_LOST_FOCUS:
        break;
    }
}
static int32_t handleInputEvent(struct android_app * /*app*/, AInputEvent * inputEvent) {
    return ImGui_ImplAndroid_HandleInputEvent(inputEvent);
}

static void drawLoop() {
    // State
    static bool showDemoWindow = true;
    static bool showAnotherWindow = false;
    static bool showCamera = true;

    static ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        static float f = 0.0f;
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

        // ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
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
        if (ImGui::Button("Turn off")) {
            cxx::AndroidLogger::logInfo("mainLoopStep: %s", "Close Camera");
            cxx::MainActivity::get()->closeCamera();
        }
        if (ImGui::Button("Turn on")) {
            cxx::AndroidLogger::logInfo("mainLoopStep: %s", "Open Camera");
            cxx::MainActivity::get()->openCamera();
        }
        if (ImGui::Button("Close Me")) {
            showCamera = false;
        }
        ImGui::End();
    }
}

void android_main(struct android_app * app) { // NOLINT(readability-identifier-naming)
    cxx::AndroidLogger::logInfo("android_main: %s", "start");

    app->onAppCmd = handleAppCmd;
    app->onInputEvent = handleInputEvent;

    while (true)
    {
        int outEvents;
        struct android_poll_source * outData;

        // Poll all events. If the app is not visible, this loop blocks until gInitialized == true.
        while (ALooper_pollOnce(cxx::MainActivity::get()->isInitialized() ? 0 : -1, nullptr, &outEvents, (void **)&outData) >= 0)
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
                if (!cxx::MainActivity::get()->isInitialized()) {
                    cxx::MainActivity::get()->shutdown();
                }

                return;
            }
        }

        // Initiate a new frame
        cxx::MainActivity::get()->mainLoopStep(drawLoop);
    }
}
