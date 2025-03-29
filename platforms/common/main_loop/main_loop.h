#pragma once

#include <platforms/common/camera/i_camera.h>

#include <memory>

#include "context.h"

namespace cxx {
    /**
     * @brief MainLoop class
     */
    class MainLoop final {
    public:
        MainLoop(std::shared_ptr< ICamera > camera);
        ~MainLoop() = default;

        void draw(const std::shared_ptr< Context > & context);

    private:
        const std::shared_ptr< ICamera > camera_;

        // State settings
        bool fff_ = true;
        bool showDemoWindow_ = true;
        bool showAnotherWindow_ = false;

        ImVec4 clearColor_ = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    };
} // namespace cxx
