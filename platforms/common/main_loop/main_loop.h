#pragma once

#include <platforms/common/camera/i_camera.h>
#include <platforms/common/client/interface/i_greeter_client.h>

#include <memory>

#include "context.h"

namespace cxx {
    /**
     * @brief MainLoop class
     */
    class MainLoop final {
    public:
        explicit MainLoop(
         std::shared_ptr< ICamera > camera,
         std::shared_ptr< IReceiptScannerClient > client);
        ~MainLoop() = default;

        void draw(const std::shared_ptr< Context > & context);

    private:
        const std::shared_ptr< ICamera > camera_;
        const std::shared_ptr< IReceiptScannerClient > client_;

        // State settings
        bool fff_ = true;
        bool showDemoWindow_ = true;
        bool showAnotherWindow_ = false;

        ImVec4 clearColor_ = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    };
} // namespace cxx
