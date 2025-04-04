#pragma once

#include "i_camera.h"

namespace cxx {

    class NullCamera final: public ICamera {
    public:
        NullCamera();
        ~NullCamera() override = default;

        void openCamera() override {}
        void closeCamera() override {}
    };

} // namespace cxx
