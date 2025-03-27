#pragma once

#include <memory>
#include <mutex>

namespace cxx {
    class Camera {
    public:
        class ISource {
            virtual std::shared_ptr< int8_t > getLast() = 0;
        };

        struct Frame {
            const std::unique_ptr< int8_t > data;
            const int width;
            const int height;
            const int channels;
        };

    public:
        Camera() = default;
        virtual ~Camera() = default;

        std::shared_ptr< Frame > lastFrame();

        void connectSource(std::shared_ptr< ISource > source);
        void clearSource();

    protected:
        std::shared_ptr< Frame > lastFrame_;
    };
} // namespace cxx
