#pragma once

#include <memory>

#include "camera_sink.h"
#include "frame.h"

namespace cxx {

    class ICamera {
    public:
        explicit ICamera(std::shared_ptr< CameraSink > cameraSink);
        virtual ~ICamera() = default;

        std::shared_ptr< Frame > lastFrame();

        virtual void openCamera() = 0;
        virtual void closeCamera() = 0;

    protected:
        std::shared_ptr< CameraSink > cameraSink_;
    };

} // namespace cxx
