#include "main_loop.h"

#include <GLES3/gl3.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/core/mat.hpp>
#include <opencv4/opencv2/objdetect.hpp>
#include <opencv4/opencv2/opencv.hpp>

using namespace cxx;

static std::mutex mutex;
static std::atomic_bool busy = false;
static bool lastBool = false;
static cv::Mat lastCorners;
static std::string lastResult = "";
constexpr int cropSize = 256;

MainLoop::MainLoop(std::shared_ptr< ICamera > camera)
  : camera_(std::move(camera)) {
}

void MainLoop::draw(const std::shared_ptr< Context > & context) {
    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (false && showDemoWindow_) { // NOLINT
        ImGui::ShowDemoWindow(&showDemoWindow_);
    }

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    if (false) // NOLINT
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");         // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &showDemoWindow_); // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &showAnotherWindow_);
        // ImGui::Checkbox("Camera", &showCamera);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);             // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float *)&clearColor_); // Edit 3 floats representing a color

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
    if (false && showAnotherWindow_) // NOLINT
    {
        ImGui::Begin("Another Window", &showAnotherWindow_); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me")) {
            showAnotherWindow_ = false;
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
            SPDLOG_ERROR("MainCppCameraHelper: %s", "Failed to generate OpenGL texture!");
        }
    }
    ImGuiViewport * viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(0, context->statusBarHeight.value_or(0)), ImGuiCond_Always);
    // ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGui::StyleColorsLight(&ImGui::GetStyle());

    ImGui::PushStyleColor(ImGuiCol_WindowBg, context->backgroudColor);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("Camera Window", &fff_, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);

    // ImVec2 vMin = ImGui::GetWindowContentRegionMin();
    // ImVec2 vMax = ImGui::GetWindowContentRegionMax();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();

    ImGui::Text("Hello from another window!");
    static bool rotateImg = true;
    ImGui::Checkbox("Rotate", &rotateImg);
    if (auto lastFrame = camera_->lastFrame(); lastFrame && textureId) {
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        if (rotateImg && !lastFrame->isVertical()) {
            lastFrame->rotate();
        }
        if (!busy) {
            busy = true;
            auto t = std::thread([frame = lastFrame]() mutable {
                cv::UMat bgMat;
                {
                    cv::Mat rgbMat(frame->height, frame->width, CV_MAKETYPE(CV_8U, frame->channels), frame->data.get());
                    const int offsetW = (rgbMat.cols - cropSize) / 2;
                    const int offsetH = (rgbMat.rows - cropSize) / 2;
                    const cv::Rect roi(offsetW, offsetH, cropSize, cropSize);
                    cv::cvtColor(rgbMat(roi), bgMat, cv::COLOR_RGBA2GRAY);
                    frame.reset();
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
        static std::weak_ptr< cxx::Frame > lastWeak;

        static int offsetW;
        static int offsetH;

        if (lastWeak.expired()) {
            lastWeak = lastFrame;
            cv::Mat bgMat;
            cv::Mat rgbMat(lastFrame->height, lastFrame->width, CV_MAKETYPE(CV_8U, lastFrame->channels), lastFrame->data.get());
            offsetW = (rgbMat.cols - cropSize) / 2;
            offsetH = (rgbMat.rows - cropSize) / 2;
            const cv::Rect roi(offsetW, offsetH, cropSize, cropSize);
            cv::cvtColor(rgbMat, bgMat, cv::COLOR_RGBA2GRAY);

            // cv::Mat tmp = rgbMat(roi).clone();
            // rgbMat /= 0.7;
            // tmp.copyTo(rgbMat(roi));

            // rgbMat.copyTo(rgbMat);

            if (std::lock_guard< std::mutex > lock(mutex); !lastCorners.empty()) {
                std::vector< cv::Point > qrPoints;
                qrPoints.reserve(4);
                for (int i = 0; i < 4; i++) {
                    qrPoints.push_back(cv::Point(offsetW + lastCorners.at< float >(0, i * 2), offsetH + lastCorners.at< float >(0, i * 2 + 1)));
                }

                cv::polylines(bgMat, qrPoints, true, cv::Scalar(0, 0, 255), 3); // Красный цвет, толщина 3 пикселя
            }

            // memcpy(data.get(), rotatedMat.data, width * height * channels);

            glBindTexture(GL_TEXTURE_2D, textureId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, lastFrame->width, lastFrame->height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, bgMat.data);

            glBindTexture(GL_TEXTURE_2D, textureId2);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cropSize, cropSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbMat(roi).clone().data);
        }

        ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::Image(textureId, ImVec2(lastFrame->width, lastFrame->height));
        ImGui::GetWindowDrawList()->AddImage(textureId2, ImVec2(p.x + offsetW, p.y + offsetH), ImVec2(p.x + offsetW + cropSize, p.y + offsetH + cropSize), ImVec2(0, 0), ImVec2(1, 1));
        ImGui::GetWindowDrawList()->AddRect(ImVec2(p.x + offsetW, p.y + offsetH), ImVec2(p.x + offsetW + cropSize, p.y + offsetH + cropSize), ImColor(215, 215, 215), 0, 2.0f);
        ImGui::Image(textureId2, ImVec2(cropSize, cropSize));
        ImGui::Text("pointer = %x", textureId);
        ImGui::Text("size = %d x %d", lastFrame->width, lastFrame->height);
    }
    // ImGui::PushItemWidth(-1.0f);
    if (ImGui::Button("Turn off", ImVec2(-1.0f, 0.0f))) {
        SPDLOG_INFO("mainLoopStep: %s", "Close Camera");
        camera_->closeCamera();
    }
    if (ImGui::Button("Turn on", ImVec2(-1.0f, 0.0f))) {
        SPDLOG_INFO("mainLoopStep: %s", "Open Camera");
        camera_->openCamera();
    }
    // ImGui::PopItemWidth();
    ImGui::End();
}
