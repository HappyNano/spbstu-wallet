#pragma once

#include <platforms/common/camera/i_camera.h>
#include <platforms/common/client/finance_client.h>

#include <memory>

#include "context.h"

namespace cxx {
    /**
     * @brief MainLoop class
     */
    class MainLoop final {
    public:
        explicit MainLoop(
         std::shared_ptr<ICamera> camera,
         std::shared_ptr<FinanceClient> client);
        ~MainLoop() = default;

        void draw(const std::shared_ptr<Context>& context);

    private:
        const std::shared_ptr<ICamera> camera_;
        const std::shared_ptr<FinanceClient> client_;

        // State settings
        bool fff_ = true;
        bool showDemoWindow_ = true;
        bool showAnotherWindow_ = false;

        ImVec4 clearColor_ = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        // UI State variables
        std::string userToken_;
        bool isAuthenticated_ = false;
        bool showLoginScreen_ = true;
        bool showQRScanScreen_ = false;
        bool showReceiptDetails_ = false;
        bool showTransactionsList_ = false;
        bool showStatistics_ = false;

        // Receipt and transaction data
        wallet::ReceiptDetailsResponse currentReceiptDetails_;
        wallet::TransactionsResponse currentTransactions_;
        wallet::StatisticsResponse currentStatistics_;

        // Device info for authentication
        std::string deviceId_ = "mobile_device";
        std::string deviceName_ = "Android Device";

        // Draw UI components
        void drawLoginScreen();
        void drawQRScanScreen();
        void drawReceiptDetails();
        void drawTransactionsList();
        void drawStatistics();
        void drawNavigation();
    };
} // namespace cxx
