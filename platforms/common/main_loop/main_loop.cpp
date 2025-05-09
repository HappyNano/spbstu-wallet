#include "main_loop.h"
#include "platforms/common/client/interface/i_greeter_client.h"

// include for android build
#ifdef __ANDROID__
#include <GLES3/gl3.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

#include <imgui.h>
#include <imgui_stdlib.h>
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

MainLoop::MainLoop(
 std::shared_ptr< ICamera > camera,
 std::shared_ptr< IReceiptScannerClient > client)
  : camera_(std::move(camera))
  , client_(std::move(client)) {
}

void MainLoop::draw(const std::shared_ptr< Context > & context) {
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
    ImGui::SetNextWindowPos(ImVec2(0, context->getStatusBarHeight().value_or(0)), ImGuiCond_Always);
    // ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGui::StyleColorsLight(&ImGui::GetStyle());

    ImGui::PushStyleColor(ImGuiCol_WindowBg, context->getBackgroundColor().value_or(ImVec4{ 1, 1, 1, 1 }));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("Camera Window", &fff_, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);

    // ImVec2 vMin = ImGui::GetWindowContentRegionMin();
    // ImVec2 vMax = ImGui::GetWindowContentRegionMax();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();

    ImGui::Text("Hello from another window!");
    static bool getResponse = false;
    static bool showCamera = true;
    static IReceiptScannerClient::Response lastResponse;
    if (auto lastFrame = camera_->lastFrame(); showCamera && lastFrame && textureId) {
        static bool rotateImg = true;
        ImGui::Checkbox("Rotate", &rotateImg);
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
            if (!lastResult.empty()) {
                lastResponse = client_->ProcessQRCode("testuser", lastResult);
                showCamera = false;
                getResponse = true;
            }
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

        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - lastFrame->width * 1.5f) * 0.5f);
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::Image(textureId, ImVec2(lastFrame->width * 1.5f, lastFrame->height * 1.5f));
        ImGui::GetWindowDrawList()->AddImage(
         textureId2,
         ImVec2(p.x + offsetW * 1.5, p.y + offsetH * 1.5),
         ImVec2(p.x + offsetW * 1.5 + cropSize * 1.5, p.y + offsetH * 1.5 + cropSize * 1.5),
         ImVec2(0, 0),
         ImVec2(1, 1));
        ImGui::GetWindowDrawList()->AddRect(
         ImVec2(p.x + offsetW * 1.5, p.y + offsetH * 1.5),
         ImVec2(p.x + offsetW * 1.5 + cropSize * 1.5, p.y + offsetH * 1.5 + cropSize * 1.5),
         ImColor(215, 215, 215),
         0,
         2.0f);
    }

    if (getResponse) {
        if (!lastResponse.error.empty()) {
            ImGui::Text("Error: %s", lastResponse.error.c_str());
        } else {
            ImGui::Text("t = %s", lastResponse.t.c_str());
            ImGui::Text("s = %f", lastResponse.s);
            ImGui::Text("fn = %s", lastResponse.fn.c_str());
            ImGui::Text("i = %s", lastResponse.i.c_str());
            ImGui::Text("fp = %s", lastResponse.fp.c_str());
            ImGui::Text("n = %d", lastResponse.n);
        }
    }

    if (ImGui::Button("Turn off", ImVec2(-1.0f, 0.0f))) {
        SPDLOG_INFO("mainLoopStep: %s", "Close Camera");
        camera_->closeCamera();
        showCamera = false;
    }
    if (ImGui::Button("Turn on", ImVec2(-1.0f, 0.0f))) {
        SPDLOG_INFO("mainLoopStep: %s", "Open Camera");
        camera_->openCamera();
        showCamera = true;
        getResponse = false;
        lastResult.clear();
    }
    // ImGui::PopItemWidth();
    ImGui::End();
}
