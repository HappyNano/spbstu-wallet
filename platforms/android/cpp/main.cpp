// dear imgui: standalone example application for Android + OpenGL ES 3

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/asset_manager.h>
#include <android_native_app_glue.h>
#include <atomic>
#include <imgui.h>
#include <imgui_impl_android.h>
#include <imgui_impl_opengl3.h>

#include <opencv2/core/mat.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include <platforms/android/cpp/camera/camera.h>
#include <platforms/android/cpp/logger/logger.h>
#include <platforms/android/cpp/main_activity/main_activity.h>

#include <mutex>
#include <thread>

static std::mutex mutex;
static std::atomic_bool busy = false;
static bool lastBool = false;
static cv::Mat lastCorners;
static std::string lastResult = "";
static constexpr auto BACKGROUD_COLOR = ImVec4(217 / 255.f, 219 / 255.f, 218 / 255.f, 1.0f);

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
    static bool fff = true;
    static bool showDemoWindow = true;
    static bool showAnotherWindow = false;

    static ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (false && showDemoWindow) { // NOLINT
        ImGui::ShowDemoWindow(&showDemoWindow);
    }

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    if (false) // NOLINT
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");        // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &showDemoWindow); // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &showAnotherWindow);
        // ImGui::Checkbox("Camera", &showCamera);

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
    if (false && showAnotherWindow) // NOLINT
    {
        ImGui::Begin("Another Window", &showAnotherWindow); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me")) {
            showAnotherWindow = false;
        }
        ImGui::End();
    }

    static GLuint textureId = 0;
    static GLuint textureId2 = 0;
    if (!textureId) {
        // Генерация текстуры
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        if (!glIsTexture(textureId)) {
            cxx::AndroidLogger::logError("MainCppCameraHelper: %s", "Failed to generate OpenGL texture!");
        }
    }
    ImGuiViewport * viewport = ImGui::GetMainViewport();
    int statusBarHeight = cxx::MainActivity::get()->getStatusBarHeight();
    ImGui::SetNextWindowPos(ImVec2(0, statusBarHeight), ImGuiCond_Always);
    // ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGui::StyleColorsLight(&ImGui::GetStyle());

    ImGui::PushStyleColor(ImGuiCol_WindowBg, BACKGROUD_COLOR);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("Camera Window", &fff, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);

    // ImVec2 vMin = ImGui::GetWindowContentRegionMin();
    // ImVec2 vMax = ImGui::GetWindowContentRegionMax();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();

    ImGui::Text("Hello from another window!");
    static bool rotateImg = true;
    ImGui::Checkbox("Rotate", &rotateImg);
    if (auto lastTexture = cxx::CameraHelper::get()->last(); lastTexture && textureId) {
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        if (rotateImg && !lastTexture->isVertical()) {
            lastTexture->rotate();
        }
        if (!busy) {
            busy = true;
            auto t = std::thread([texture = lastTexture]() mutable {
                cv::UMat bgMat;
                {
                    cv::Mat rgbMat(texture->height, texture->width, CV_MAKETYPE(CV_8U, texture->channels), texture->data.get());
                    const int cropSize = 256;
                    const int offsetW = (rgbMat.cols - cropSize) / 2;
                    const int offsetH = (rgbMat.rows - cropSize) / 2;
                    const cv::Rect roi(offsetW, offsetH, cropSize, cropSize);
                    cv::cvtColor(rgbMat(roi), bgMat, cv::COLOR_RGBA2GRAY);
                    texture.reset();
                }
                static auto qrDet = cv::QRCodeDetectorAruco();

                cv::Mat corners;
                bool detectResult = qrDet.detect(bgMat, corners);
                {
                    std::lock_guard< std::mutex > lock(mutex);
                    lastBool = detectResult;
                    lastCorners = corners;
                }

                if (detectResult) {
                    cv::Mat points, rectImage;
                    std::string decodeResult = qrDet.detectAndDecode(bgMat, points, rectImage);
                    {
                        std::lock_guard< std::mutex > lock(mutex);
                        lastResult = std::move(decodeResult);
                    }
                }

                busy = false;
            });
            t.detach();
        }

        {
            std::lock_guard< std::mutex > lock(mutex);
            ImGui::Text("detectionResult = %i", lastBool);
            ImGui::Text("Result = \'%s\'", lastResult.c_str());
        }

        const int cropSize = 256;
        static std::weak_ptr< cxx::Texture > lastWeak;
        if (lastWeak.expired()) {
            lastWeak = lastTexture;
            cv::Mat bgMat;
            cv::Mat rgbMat(lastTexture->height, lastTexture->width, CV_MAKETYPE(CV_8U, lastTexture->channels), lastTexture->data.get());
            const int offsetW = (rgbMat.cols - cropSize) / 2;
            const int offsetH = (rgbMat.rows - cropSize) / 2;
            const cv::Rect roi(offsetW, offsetH, cropSize, cropSize);
            cv::cvtColor(rgbMat(roi), bgMat, cv::COLOR_RGBA2GRAY);

            if (std::lock_guard< std::mutex > lock(mutex); !lastCorners.empty()) {
                std::vector< cv::Point > qrPoints;
                qrPoints.reserve(4);
                for (int i = 0; i < 4; i++) {
                    qrPoints.push_back(cv::Point(offsetW + lastCorners.at< float >(0, i * 2), offsetH + lastCorners.at< float >(0, i * 2 + 1)));
                }

                cv::polylines(rgbMat, qrPoints, true, cv::Scalar(0, 0, 255), 3); // Красный цвет, толщина 3 пикселя
            }

            // memcpy(data.get(), rotatedMat.data, width * height * channels);

            glBindTexture(GL_TEXTURE_2D, textureId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, lastTexture->width, lastTexture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbMat.data);

            glBindTexture(GL_TEXTURE_2D, textureId2);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, cropSize, cropSize, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, bgMat.data);
        }

        ImGui::Image(textureId, ImVec2(lastTexture->width, lastTexture->height));
        ImGui::Image(textureId2, ImVec2(cropSize, cropSize));
        ImGui::Text("pointer = %x", textureId);
        ImGui::Text("size = %d x %d", lastTexture->width, lastTexture->height);
    }
    // ImGui::PushItemWidth(-1.0f);
    if (ImGui::Button("Turn off", ImVec2(-1.0f, 0.0f))) {
        cxx::AndroidLogger::logInfo("mainLoopStep: %s", "Close Camera");
        cxx::MainActivity::get()->closeCamera();
    }
    if (ImGui::Button("Turn on", ImVec2(-1.0f, 0.0f))) {
        cxx::AndroidLogger::logInfo("mainLoopStep: %s", "Open Camera");
        cxx::MainActivity::get()->openCamera();
    }
    // ImGui::PopItemWidth();
    ImGui::End();
}

void android_main(struct android_app * app) { // NOLINT(readability-identifier-naming)
    cxx::AndroidLogger::logInfo("android_main: %s", "start");

    app->onAppCmd = handleAppCmd;
    app->onInputEvent = handleInputEvent;

    cxx::MainActivity::get()->setBackgroudColor(BACKGROUD_COLOR);

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
