#pragma once

#include <jni.h>

#include <platforms/common/camera/camera_sink.h>
#include <utils/singleton/singleton.h>

namespace cxx {

    class JniCameraSink final
      : public CameraSink,
        public util::Singleton< JniCameraSink > {
    public:
        JniCameraSink() = default;
        ~JniCameraSink() override = default;

        /**
         * @brief Copies frame data from Java make new frame
         *
         * @param data image data
         * @param width image width
         * @param height image height
         * @param channels image channels (for rgb = 3)
         */
        void loadNewFrameFromJni(jbyte * data, int width, int height, int channels);
    };

} // namespace cxx
