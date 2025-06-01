#include "main_loop.h"

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
#include <opencv4/opencv2/objdetect.hpp>
#include <opencv4/opencv2/opencv.hpp>

#include <google/protobuf/timestamp.pb.h>

using namespace cxx;

MainLoop::MainLoop(
 std::shared_ptr< ICamera > camera,
 std::shared_ptr< FinanceClient > client)
  : camera_(std::move(camera))
  , client_(std::move(client)) {
}

void MainLoop::draw(const std::shared_ptr< Context > & context) {
    ImGuiViewport * viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(0, context->getStatusBarHeight().value_or(0)), ImGuiCond_Always);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGui::StyleColorsLight(&ImGui::GetStyle());

    ImGui::PushStyleColor(ImGuiCol_WindowBg, context->getBackgroundColor().value_or(ImVec4{ 1, 1, 1, 1 }));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("Finance App", &fff_, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();

    // Main application flow
    if (!isAuthenticated_) {
        drawLoginScreen();
    } else {
        // Draw the navigation bar
        drawNavigation();

        // Draw the current active screen
        if (showQRScanScreen_) {
            drawQRScanScreen();
        } else if (showReceiptDetails_) {
            drawReceiptDetails();
        } else if (showTransactionsList_) {
            drawTransactionsList();
        } else if (showStatistics_) {
            drawStatistics();
        }
    }

    ImGui::End();
}

void MainLoop::drawLoginScreen() {
    ImGui::SetCursorPos(ImVec2((ImGui::GetWindowSize().x - 300) * 0.5f, (ImGui::GetWindowSize().y - 200) * 0.5f));

    ImGui::BeginChild("LoginPanel", ImVec2(300, 200), true, ImGuiWindowFlags_NoScrollbar);

    ImGui::SetCursorPosY(20);
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("Finance App Login").x) * 0.5f);
    ImGui::Text("Finance App Login");

    ImGui::Spacing();
    ImGui::Spacing();

    // Device ID input
    ImGui::SetCursorPosX(10);
    ImGui::Text("Device ID:");
    ImGui::SetCursorPosX(10);
    ImGui::SetNextItemWidth(ImGui::GetWindowSize().x - 20);
    ImGui::InputText("##deviceid", &deviceId_);

    ImGui::Spacing();

    // Device Name input
    ImGui::SetCursorPosX(10);
    ImGui::Text("Device Name:");
    ImGui::SetCursorPosX(10);
    ImGui::SetNextItemWidth(ImGui::GetWindowSize().x - 20);
    ImGui::InputText("##devicename", &deviceName_);

    ImGui::Spacing();
    ImGui::Spacing();

    // Login button
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 100) * 0.5f);
    if (ImGui::Button("Login", ImVec2(100, 30))) {
        auto response = client_->Authenticate(deviceId_, deviceName_);

        if (response.has_token()) {
            userToken_ = response.token();
            isAuthenticated_ = true;
            showQRScanScreen_ = true;
            SPDLOG_INFO("User authenticated with token: " + userToken_);
        } else if (response.has_error()) {
            SPDLOG_ERROR("Authentication error: " + response.error().message());
        }
    }

    ImGui::EndChild();
}

void MainLoop::drawNavigation() {
    // Top navigation bar
    ImGui::BeginChild("NavigationBar", ImVec2(ImGui::GetWindowSize().x, 50), true);

    float buttonWidth = ImGui::GetWindowSize().x / 5.0f;

    if (ImGui::Button("Отчет", ImVec2(buttonWidth, 40))) {
        showQRScanScreen_ = false;
        showReceiptDetails_ = false;
        showTransactionsList_ = false;
        showStatistics_ = true;

        // Fetch statistics data
        currentStatistics_ = client_->GetStatistics(userToken_);
    }

    ImGui::SameLine();
    if (ImGui::Button("Транзакции", ImVec2(buttonWidth, 40))) {
        showQRScanScreen_ = false;
        showReceiptDetails_ = false;
        showTransactionsList_ = true;
        showStatistics_ = false;

        // Fetch transactions data
        currentTransactions_ = client_->GetTransactions(userToken_);
    }

    ImGui::SameLine();
    if (ImGui::Button("Чек", ImVec2(buttonWidth, 40))) {
        showQRScanScreen_ = true;
        showReceiptDetails_ = false;
        showTransactionsList_ = false;
        showStatistics_ = false;

        // Enable camera for QR scanning
        camera_->openCamera();
    }

    ImGui::SameLine();
    if (ImGui::Button("Статистика", ImVec2(buttonWidth, 40))) {
        showQRScanScreen_ = false;
        showReceiptDetails_ = false;
        showTransactionsList_ = false;
        showStatistics_ = true;

        // Fetch statistics data
        currentStatistics_ = client_->GetStatistics(userToken_);
    }

    ImGui::SameLine();
    if (ImGui::Button("Настройки", ImVec2(buttonWidth, 40))) {
        // Show settings screen logic would go here
    }

    ImGui::EndChild();
}

void MainLoop::drawQRScanScreen() {
    static GLuint textureId = 0;
    static GLuint textureId2 = 0;
    if (!textureId) {
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        if (!glIsTexture(textureId)) {
            SPDLOG_ERROR("MainCppCameraHelper: Failed to generate OpenGL texture!");
        }

        glGenTextures(1, &textureId2);
    }

    static std::mutex mutex;
    static std::atomic_bool busy = false;
    static bool lastBool = false;
    static cv::Mat lastCorners;
    static std::string lastResult = "";
    constexpr int cropSize = 256;

    ImGui::BeginChild("QRScanArea", ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y - 50), false);

    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("Сканирование QR-кода").x) * 0.5f);
    ImGui::Text("Сканирование QR-кода");

    if (auto lastFrame = camera_->lastFrame(); lastFrame && textureId) {
        static bool rotateImg = true;
        ImGui::Checkbox("Повернуть изображение", &rotateImg);

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
            ImGui::Text("Обнаружен QR-код: %s", lastBool ? "Да" : "Нет");

            if (!lastResult.empty()) {
                ImGui::Text("QR содержимое: %s", lastResult.c_str());

                if (ImGui::Button("Обработать QR-код", ImVec2(200, 30))) {
                    // Process QR code with the finance service
                    auto timestamp = google::protobuf::Timestamp();
                    auto now = std::chrono::system_clock::now();
                    auto seconds = std::chrono::duration_cast< std::chrono::seconds >(now.time_since_epoch()).count();
                    auto nanos = std::chrono::duration_cast< std::chrono::nanoseconds >(now.time_since_epoch()).count() % 1000000000;
                    timestamp.set_seconds(seconds);
                    timestamp.set_nanos(static_cast< int32_t >(nanos));

                    currentReceiptDetails_ = client_->ProcessQRCode(userToken_, lastResult, timestamp);

                    if (!currentReceiptDetails_.has_error()) {
                        showQRScanScreen_ = false;
                        showReceiptDetails_ = true;
                        camera_->closeCamera();
                    } else {
                        SPDLOG_ERROR("Error processing QR code: " + currentReceiptDetails_.error().message());
                    }

                    lastResult.clear();
                }
            }
        }

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

            if (std::lock_guard< std::mutex > lock(mutex); !lastCorners.empty()) {
                std::vector< cv::Point > qrPoints;
                qrPoints.reserve(4);
                for (int i = 0; i < 4; i++) {
                    qrPoints.push_back(cv::Point(offsetW + lastCorners.at< float >(0, i * 2), offsetH + lastCorners.at< float >(0, i * 2 + 1)));
                }

                cv::polylines(bgMat, qrPoints, true, cv::Scalar(0, 0, 255), 3);
            }

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

    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 200) * 0.5f);
    if (ImGui::Button("Отключить камеру", ImVec2(200, 30))) {
        SPDLOG_INFO("mainLoopStep: Close Camera");
        camera_->closeCamera();
    }

    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 200) * 0.5f);
    if (ImGui::Button("Включить камеру", ImVec2(200, 30))) {
        SPDLOG_INFO("mainLoopStep: Open Camera");
        camera_->openCamera();
        lastResult.clear();
    }

    ImGui::EndChild();
}

void MainLoop::drawReceiptDetails() {
    ImGui::BeginChild("ReceiptDetailsArea", ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y - 50), false);

    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("Детали чека").x) * 0.5f);
    ImGui::Text("Детали чека");
    ImGui::Separator();

    if (currentReceiptDetails_.has_receipt_data()) {
        const auto & receipt = currentReceiptDetails_.receipt_data();

        ImGui::Text("Информация о чеке:");
        ImGui::Separator();

        if (receipt.has_receipt()) {
            const auto & r = receipt.receipt();

            // Display basic receipt info
            // ImGui::Text("ИНН: {}", r.inn().c_str());
            ImGui::Text("Дата: %s", r.t().c_str());
            ImGui::Text("Сумма: %s руб.", std::to_string(r.s() / 100.0f).c_str());

            if (receipt.has_retailer()) {
                ImGui::Text("Продавец: %s", receipt.retailer().name().c_str());
            }

            // Display items
            if (receipt.items_size() > 0) {
                ImGui::Separator();
                ImGui::Text("Товары:");
                ImGui::Separator();

                ImGui::BeginChild("ItemsList", ImVec2(ImGui::GetWindowSize().x - 20, 200), true);

                for (int i = 0; i < receipt.items_size(); i++) {
                    const auto & item = receipt.items(i);
                    ImGui::Text("%d. %s", i + 1, item.name().c_str());
                    ImGui::SameLine(ImGui::GetWindowSize().x - 150);
                    ImGui::Text("%.2f x %f = %.2f руб.", item.price() / 100.0f, item.quantity(), item.sum() / 100.0f);
                }

                ImGui::EndChild();
            }
        }
    }

    // Create transaction button
    ImGui::Separator();

    static int selectedCategoryId = 0;
    static std::vector< wallet::CategoryInfo > categories;
    static bool categoriesLoaded = false;

    if (!categoriesLoaded) {
        auto categoriesResponse = client_->GetCategories(userToken_);
        if (categoriesResponse.has_categories_list()) {
            const auto & categoryList = categoriesResponse.categories_list();
            categories.clear();
            for (int i = 0; i < categoryList.categories_size(); i++) {
                categories.push_back(categoryList.categories(i));
            }
            categoriesLoaded = true;
        }
    }

    // Category selection
    if (!categories.empty()) {
        ImGui::Text("Категория:");
        ImGui::SameLine();

        if (ImGui::BeginCombo("##categoryCombo", selectedCategoryId == 0 ? "Выберите категорию" : categories[selectedCategoryId - 1].name().c_str())) {

            for (size_t i = 0; i < categories.size(); i++) {
                bool isSelected = (selectedCategoryId == static_cast< int >(i + 1));
                if (ImGui::Selectable(categories[i].name().c_str(), isSelected)) {
                    selectedCategoryId = i + 1;
                }

                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }
    }

    // Comment field
    static std::string transactionComment;
    ImGui::Text("Комментарий:");
    ImGui::SameLine();
    ImGui::InputText("##comment", &transactionComment);

    // Create transaction
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 250) * 0.5f);
    if (ImGui::Button("Создать транзакцию", ImVec2(250, 30))) {
        const auto & receipt = currentReceiptDetails_.receipt_data();
        wallet::TransactionData transaction;
        transaction.set_type(1); // расход

        if (receipt.has_receipt()) {
            const auto & r = receipt.receipt();
            transaction.set_amount(r.s());

            // Set timestamp from receipt date if possible
            if (!r.t().empty()) {
                // Parse date_time format (ггггммддTччммcc)
                std::string dateStr = r.t();
                if (dateStr.size() >= 15) {
                    int year = std::stoi(dateStr.substr(0, 4));
                    int month = std::stoi(dateStr.substr(4, 2));
                    int day = std::stoi(dateStr.substr(6, 2));
                    int hour = std::stoi(dateStr.substr(9, 2));
                    int minute = std::stoi(dateStr.substr(11, 2));
                    int second = std::stoi(dateStr.substr(13, 2));

                    struct tm timeinfo = {};
                    timeinfo.tm_year = year - 1900;
                    timeinfo.tm_mon = month - 1;
                    timeinfo.tm_mday = day;
                    timeinfo.tm_hour = hour;
                    timeinfo.tm_min = minute;
                    timeinfo.tm_sec = second;

                    time_t rawtime = mktime(&timeinfo);
                    google::protobuf::Timestamp timestamp;
                    timestamp.set_seconds(rawtime);
                    timestamp.set_nanos(0);
                    transaction.set_allocated_timestamp(new google::protobuf::Timestamp(timestamp));
                }
            }
        } else {
            // Fallback to current time if receipt doesn't have a date
            auto now = std::chrono::system_clock::now();
            auto seconds = std::chrono::duration_cast< std::chrono::seconds >(now.time_since_epoch()).count();
            auto nanos = std::chrono::duration_cast< std::chrono::nanoseconds >(now.time_since_epoch()).count() % 1000000000;
            google::protobuf::Timestamp timestamp;
            timestamp.set_seconds(seconds);
            timestamp.set_nanos(static_cast< int32_t >(nanos));
            transaction.set_allocated_timestamp(new google::protobuf::Timestamp(timestamp));
        }

        if (selectedCategoryId > 0) {
            transaction.set_category_id(categories[selectedCategoryId - 1].id());
        }

        if (!transactionComment.empty()) {
            transaction.set_comment(transactionComment);
        }

        // Call gRPC service to create transaction
        auto response = client_->CreateTransaction(userToken_, transaction);

        if (response.has_transaction_id()) {
            ImGui::OpenPopup("TransactionCreated");
        } else if (response.has_error()) {
            ImGui::OpenPopup("TransactionError");
        }
    }

    // Transaction created popup
    if (ImGui::BeginPopupModal("TransactionCreated", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Транзакция успешно создана!");

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            showReceiptDetails_ = false;
            showTransactionsList_ = true;
            currentTransactions_ = client_->GetTransactions(userToken_);
        }
        ImGui::EndPopup();
    }

    // Transaction error popup
    if (ImGui::BeginPopupModal("TransactionError", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Ошибка при создании транзакции!");

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    } else if (currentReceiptDetails_.has_error()) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Ошибка: %s", currentReceiptDetails_.error().message().c_str());
    }

    // Back button
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 120) * 0.5f);
    if (ImGui::Button("Назад", ImVec2(120, 30))) {
        showReceiptDetails_ = false;
        showQRScanScreen_ = true;
        camera_->openCamera();
    }

    ImGui::EndChild();
}

void MainLoop::drawTransactionsList() {
    ImGui::BeginChild("TransactionsArea", ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y - 50), false);

    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("Список транзакций").x) * 0.5f);
    ImGui::Text("Список транзакций");
    ImGui::Separator();

    // Filter controls
    static int transactionTypeFilter = -1; // -1: All, 0: Income, 1: Expense
    static google::protobuf::Timestamp fromDate;
    static google::protobuf::Timestamp toDate;
    static char fromDateStr[11] = ""; // YYYY-MM-DD
    static char toDateStr[11] = "";   // YYYY-MM-DD
    static int selectedCategoryId = 0;
    static std::vector< wallet::CategoryInfo > categories;

    if (categories.empty()) {
        auto categoriesResponse = client_->GetCategories(userToken_);
        if (categoriesResponse.has_categories_list()) {
            const auto & categoryList = categoriesResponse.categories_list();
            for (int i = 0; i < categoryList.categories_size(); i++) {
                categories.push_back(categoryList.categories(i));
            }
        }
    }

    ImGui::Text("Фильтры:");

    // Transaction type filter
    const char * transactionTypes[] = { "Все", "Доходы", "Расходы" };
    ImGui::SameLine();
    if (ImGui::BeginCombo("##typeCombo", transactionTypes[transactionTypeFilter + 1])) {
        for (int i = 0; i < IM_ARRAYSIZE(transactionTypes); i++) {
            bool isSelected = (i == transactionTypeFilter + 1);
            if (ImGui::Selectable(transactionTypes[i], isSelected)) {
                transactionTypeFilter = i - 1;
            }

            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    // Date filters
    ImGui::SameLine();
    ImGui::Text("С:");
    ImGui::SameLine();
    ImGui::InputText("##fromDate", fromDateStr, IM_ARRAYSIZE(fromDateStr));

    ImGui::SameLine();
    ImGui::Text("По:");
    ImGui::SameLine();
    ImGui::InputText("##toDate", toDateStr, IM_ARRAYSIZE(toDateStr));

    // Category filter
    ImGui::SameLine();
    ImGui::Text("Категория:");
    ImGui::SameLine();

    if (ImGui::BeginCombo("##categoryFilterCombo", selectedCategoryId == 0 ? "Все категории" : categories[selectedCategoryId - 1].name().c_str())) {

        if (ImGui::Selectable("Все категории", selectedCategoryId == 0)) {
            selectedCategoryId = 0;
        }

        for (size_t i = 0; i < categories.size(); i++) {
            bool isSelected = (selectedCategoryId == static_cast<int>(i + 1));
            if (ImGui::Selectable(categories[i].name().c_str(), isSelected)) {
                selectedCategoryId = i + 1;
            }

            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    // Apply filters button
    ImGui::SameLine();
    if (ImGui::Button("Применить")) {
        // Parse dates
        if (strlen(fromDateStr) == 10) { // YYYY-MM-DD
            int year = std::stoi(std::string(fromDateStr, 0, 4));
            int month = std::stoi(std::string(fromDateStr, 5, 2));
            int day = std::stoi(std::string(fromDateStr, 8, 2));

            struct tm timeinfo = {};
            timeinfo.tm_year = year - 1900;
            timeinfo.tm_mon = month - 1;
            timeinfo.tm_mday = day;

            time_t rawtime = mktime(&timeinfo);
            fromDate.set_seconds(rawtime);
            fromDate.set_nanos(0);
        } else {
            fromDate.set_seconds(0);
            fromDate.set_nanos(0);
        }

        if (strlen(toDateStr) == 10) { // YYYY-MM-DD
            int year = std::stoi(std::string(toDateStr, 0, 4));
            int month = std::stoi(std::string(toDateStr, 5, 2));
            int day = std::stoi(std::string(toDateStr, 8, 2));

            struct tm timeinfo = {};
            timeinfo.tm_year = year - 1900;
            timeinfo.tm_mon = month - 1;
            timeinfo.tm_mday = day;
            timeinfo.tm_hour = 23;
            timeinfo.tm_min = 59;
            timeinfo.tm_sec = 59;

            time_t rawtime = mktime(&timeinfo);
            toDate.set_seconds(rawtime);
            toDate.set_nanos(0);
        } else {
            toDate.set_seconds(0);
            toDate.set_nanos(0);
        }

        // Fetch transactions with filters
        int categoryIdFilter = (selectedCategoryId > 0) ? categories[selectedCategoryId - 1].id() : 0;

        currentTransactions_ = client_->GetTransactions(
         userToken_,
         fromDate,
         toDate,
         transactionTypeFilter,
         categoryIdFilter);
    }

    // Reset filters button
    ImGui::SameLine();
    if (ImGui::Button("Сбросить")) {
        transactionTypeFilter = -1;
        fromDateStr[0] = '\0';
        toDateStr[0] = '\0';
        selectedCategoryId = 0;
        fromDate.set_seconds(0);
        fromDate.set_nanos(0);
        toDate.set_seconds(0);
        toDate.set_nanos(0);

        // Fetch all transactions
        currentTransactions_ = client_->GetTransactions(userToken_);
    }

    ImGui::Separator();

    // Display transactions
    if (currentTransactions_.has_transactions()) {
        const auto & transactions = currentTransactions_.transactions();

        // Display summary
        ImGui::Text("Доходы: %.2f руб. | Расходы: %.2f руб. | Баланс: %.2f руб.", transactions.total_income() / 100.0f, transactions.total_expense() / 100.0f, transactions.balance() / 100.0f);

        ImGui::Separator();

        // Transactions table
        if (ImGui::BeginTable("TransactionsTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Дата");
            ImGui::TableSetupColumn("Категория");
            ImGui::TableSetupColumn("Сумма");
            ImGui::TableSetupColumn("Комментарий");
            ImGui::TableSetupColumn("Действия");
            ImGui::TableHeadersRow();

            for (int i = 0; i < transactions.transactions_size(); i++) {
                const auto & transaction = transactions.transactions(i);

                ImGui::TableNextRow();

                // Date
                ImGui::TableNextColumn();
                time_t timestamp = transaction.timestamp().seconds();
                struct tm * timeinfo = localtime(&timestamp);
                char buffer[20];
                strftime(buffer, sizeof(buffer), "%d.%m.%Y %H:%M", timeinfo);
                ImGui::Text("%s", buffer);

                // Category
                ImGui::TableNextColumn();
                ImGui::Text("%s", transaction.has_category_name() ? transaction.category_name().c_str() : "Без категории");

                // Amount
                ImGui::TableNextColumn();
                ImVec4 amountColor = transaction.type() == 0 ? ImVec4(0.0f, 0.7f, 0.0f, 1.0f) : ImVec4(0.7f, 0.0f, 0.0f, 1.0f);
                ImGui::TextColored(amountColor, "%.2f руб.", transaction.amount() / 100.0f);

                // Comment
                ImGui::TableNextColumn();
                ImGui::Text("%s", transaction.has_comment() ? transaction.comment().c_str() : "");

                // Actions
                ImGui::TableNextColumn();
                ImGui::PushID(transaction.id());

                if (ImGui::Button("Детали")) {
                    // Fetch transaction details
                    auto detailsResponse = client_->GetTransactionDetails(userToken_, transaction.id());
                    // Open details popup
                    ImGui::OpenPopup("TransactionDetailsPopup");
                }

                ImGui::SameLine();
                if (ImGui::Button("Удалить")) {
                    ImGui::OpenPopup("DeleteConfirmation");
                }

                // Delete confirmation popup
                if (ImGui::BeginPopupModal("DeleteConfirmation", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("Вы уверены, что хотите удалить эту транзакцию?");
                    ImGui::Separator();

                    if (ImGui::Button("Да", ImVec2(120, 0))) {
                        auto response = client_->DeleteTransaction(userToken_, transaction.id());

                        if (response.has_success()) {
                            // Refresh transactions list
                            currentTransactions_ = client_->GetTransactions(
                             userToken_,
                             fromDate,
                             toDate,
                             transactionTypeFilter,
                             (selectedCategoryId > 0) ? categories[selectedCategoryId - 1].id() : 0);
                        }

                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Нет", ImVec2(120, 0))) {
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }

                ImGui::PopID();
            }

            ImGui::EndTable();
        }

        // Pagination controls if needed
        int totalPages = (transactions.total_count() + 9) / 10; // 10 items per page
        if (totalPages > 1) {
            static int currentPage = 1;

            ImGui::Text("Страница %d из %d", currentPage, totalPages);
            ImGui::SameLine();

            if (ImGui::Button("Пред.") && currentPage > 1) {
                currentPage--;
                // Fetch with offset
                int offset = (currentPage - 1) * 10;
                currentTransactions_ = client_->GetTransactions(
                 userToken_,
                 fromDate,
                 toDate,
                 transactionTypeFilter,
                 (selectedCategoryId > 0) ? categories[selectedCategoryId - 1].id() : 0,
                 10, // limit
                 offset);
            }

            ImGui::SameLine();
            if (ImGui::Button("След.") && currentPage < totalPages) {
                currentPage++;
                // Fetch with offset
                int offset = (currentPage - 1) * 10;
                currentTransactions_ = client_->GetTransactions(
                 userToken_,
                 fromDate,
                 toDate,
                 transactionTypeFilter,
                 (selectedCategoryId > 0) ? categories[selectedCategoryId - 1].id() : 0,
                 10, // limit
                 offset);
            }
        }
    } else if (currentTransactions_.has_error()) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Ошибка: %s", currentTransactions_.error().message().c_str());
    } else {
        // Initial load
        currentTransactions_ = client_->GetTransactions(userToken_);
    }

    ImGui::EndChild();
}

void MainLoop::drawStatistics() {
    ImGui::BeginChild("StatisticsArea", ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y - 50), false);

    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("Статистика").x) * 0.5f);
    ImGui::Text("Статистика");
    ImGui::Separator();

    // Date range selection
    static char fromDateStr[11] = ""; // YYYY-MM-DD
    static char toDateStr[11] = "";   // YYYY-MM-DD

    ImGui::Text("Период:");
    ImGui::SameLine();
    ImGui::Text("С:");
    ImGui::SameLine();
    ImGui::InputText("##statsFromDate", fromDateStr, IM_ARRAYSIZE(fromDateStr));

    ImGui::SameLine();
    ImGui::Text("По:");
    ImGui::SameLine();
    ImGui::InputText("##statsToDate", toDateStr, IM_ARRAYSIZE(toDateStr));

    ImGui::SameLine();
    if (ImGui::Button("Применить")) {
        google::protobuf::Timestamp fromDate;
        google::protobuf::Timestamp toDate;

        // Parse dates
        if (strlen(fromDateStr) == 10) { // YYYY-MM-DD
            int year = std::stoi(std::string(fromDateStr, 0, 4));
            int month = std::stoi(std::string(fromDateStr, 5, 2));
            int day = std::stoi(std::string(fromDateStr, 8, 2));

            struct tm timeinfo = {};
            timeinfo.tm_year = year - 1900;
            timeinfo.tm_mon = month - 1;
            timeinfo.tm_mday = day;

            time_t rawtime = mktime(&timeinfo);
            fromDate.set_seconds(rawtime);
            fromDate.set_nanos(0);
        }

        if (strlen(toDateStr) == 10) { // YYYY-MM-DD
            int year = std::stoi(std::string(toDateStr, 0, 4));
            int month = std::stoi(std::string(toDateStr, 5, 2));
            int day = std::stoi(std::string(toDateStr, 8, 2));

            struct tm timeinfo = {};
            timeinfo.tm_year = year - 1900;
            timeinfo.tm_mon = month - 1;
            timeinfo.tm_mday = day;
            timeinfo.tm_hour = 23;
            timeinfo.tm_min = 59;
            timeinfo.tm_sec = 59;

            time_t rawtime = mktime(&timeinfo);
            toDate.set_seconds(rawtime);
            toDate.set_nanos(0);
        }

        // Fetch statistics with date range
        currentStatistics_ = client_->GetStatistics(userToken_, fromDate, toDate);
    }

    ImGui::SameLine();
    if (ImGui::Button("Сбросить")) {
        fromDateStr[0] = '\0';
        toDateStr[0] = '\0';

        // Fetch all-time statistics
        currentStatistics_ = client_->GetStatistics(userToken_);
    }

    ImGui::Separator();

    // Display statistics
    if (currentStatistics_.has_statistics()) {
        const auto & stats = currentStatistics_.statistics();

        // Summary
        ImGui::Text("Итого доходы: %.2f руб.", stats.total_income() / 100.0f);
        ImGui::Text("Итого расходы: %.2f руб.", stats.total_expense() / 100.0f);
        ImGui::Text("Баланс: %.2f руб.", stats.balance() / 100.0f);
        ImGui::Text("Кол-во транзакций доходов: %d", stats.income_transactions_count());
        ImGui::Text("Кол-во транзакций расходов: %d", stats.expense_transactions_count());

        ImGui::Separator();

        // Categories expenses
        if (stats.has_chart_data() && stats.chart_data().expenses_by_category_size() > 0) {
            ImGui::Text("Расходы по категориям:");

            if (ImGui::BeginTable("ExpensesByCategory", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Категория");
                ImGui::TableSetupColumn("Сумма");
                ImGui::TableSetupColumn("Кол-во транзакций");
                ImGui::TableSetupColumn("Процент");
                ImGui::TableHeadersRow();

                for (int i = 0; i < stats.chart_data().expenses_by_category_size(); i++) {
                    const auto & category = stats.chart_data().expenses_by_category(i);

                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", category.category_name().c_str());

                    ImGui::TableNextColumn();
                    ImGui::Text("%.2f руб.", category.total_amount() / 100.0f);

                    ImGui::TableNextColumn();
                    ImGui::Text("%d", category.transactions_count());

                    ImGui::TableNextColumn();
                    ImGui::Text("%.1f%%", category.percentage());
                }

                ImGui::EndTable();
            }
        }

        // Categories incomes
        if (stats.has_chart_data() && stats.chart_data().incomes_by_category_size() > 0) {
            ImGui::Separator();
            ImGui::Text("Доходы по категориям:");

            if (ImGui::BeginTable("IncomesByCategory", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Категория");
                ImGui::TableSetupColumn("Сумма");
                ImGui::TableSetupColumn("Кол-во транзакций");
                ImGui::TableSetupColumn("Процент");
                ImGui::TableHeadersRow();

                for (int i = 0; i < stats.chart_data().incomes_by_category_size(); i++) {
                    const auto & category = stats.chart_data().incomes_by_category(i);

                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", category.category_name().c_str());

                    ImGui::TableNextColumn();
                    ImGui::Text("%.2f руб.", category.total_amount() / 100.0f);

                    ImGui::TableNextColumn();
                    ImGui::Text("%d", category.transactions_count());

                    ImGui::TableNextColumn();
                    ImGui::Text("%.1f%%", category.percentage());
                }

                ImGui::EndTable();
            }
        }

        // Expenses by character (split transactions)
        if (stats.has_chart_data() && stats.chart_data().expenses_by_character_size() > 0) {
            ImGui::Separator();
            ImGui::Text("Расходы по персонажам:");

            if (ImGui::BeginTable("ExpensesByCharacter", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Персонаж");
                ImGui::TableSetupColumn("Сумма");
                ImGui::TableSetupColumn("Кол-во делений");
                ImGui::TableSetupColumn("Процент");
                ImGui::TableHeadersRow();

                for (int i = 0; i < stats.chart_data().expenses_by_character_size(); i++) {
                    const auto & character = stats.chart_data().expenses_by_character(i);

                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", character.character_name().c_str());

                    ImGui::TableNextColumn();
                    ImGui::Text("%.2f руб.", character.total_amount() / 100.0f);

                    ImGui::TableNextColumn();
                    ImGui::Text("%d", character.splits_count());

                    ImGui::TableNextColumn();
                    ImGui::Text("%.1f%%", character.percentage());
                }

                ImGui::EndTable();
            }
        }

        // Daily chart data
        if (stats.has_chart_data() && stats.chart_data().daily_size() > 0) {
            ImGui::Separator();
            ImGui::Text("Данные по дням:");

            if (ImGui::BeginTable("DailyData", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Дата");
                ImGui::TableSetupColumn("Доходы");
                ImGui::TableSetupColumn("Расходы");
                ImGui::TableHeadersRow();

                for (int i = 0; i < stats.chart_data().daily_size(); i++) {
                    const auto & day = stats.chart_data().daily(i);

                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", day.date().c_str());

                    ImGui::TableNextColumn();
                    ImGui::TextColored(ImVec4(0.0f, 0.7f, 0.0f, 1.0f), "%.2f руб.", day.income() / 100.0f);

                    ImGui::TableNextColumn();
                    ImGui::TextColored(ImVec4(0.7f, 0.0f, 0.0f, 1.0f), "%.2f руб.", day.expense() / 100.0f);
                }

                ImGui::EndTable();
            }
        }
    } else if (currentStatistics_.has_error()) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Ошибка: %s", currentStatistics_.error().message().c_str());
    } else {
        // Initial load
        currentStatistics_ = client_->GetStatistics(userToken_);
    }

    ImGui::EndChild();
}
