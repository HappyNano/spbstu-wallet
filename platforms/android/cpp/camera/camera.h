#pragma once

#include <platforms/common/camera/i_camera.h>
#include <platforms/android/cpp/main_activity/android_main_activity.h>

namespace cxx {

    class AndroidCamera final: public ICamera {
    public:
        explicit AndroidCamera(
         std::weak_ptr< AndroidMainActivity > mainActivity,
         std::shared_ptr< CameraSink > cameraSink);
        ~AndroidCamera() override = default;

        void openCamera() override;
        void closeCamera() override;

    private:
        std::weak_ptr< AndroidMainActivity > mainActivity_;
    };

} // namespace cxx
