#pragma once

#include <jni.h>

#include <utils/singleton/singleton.h>

#include <memory>
#include <mutex>

namespace cxx {

    class Texture final {
    public:
        using shared = std::shared_ptr< Texture >;

        Texture(jbyte * data, int width, int height, int channels);
        ~Texture();

        std::unique_ptr< jbyte > data;
        int width;
        int height;
        int channels;

        auto rotate() -> void;
        auto isVertical() const -> bool;
    };

    class CameraHelper final: public util::Singleton< CameraHelper > {
    public:
        CameraHelper() = default;
        ~CameraHelper() override = default;

        auto newTexture(jbyte * data, int width, int height, int channels) -> void;
        auto last() const -> Texture::shared;

    private:
        Texture::shared lastTexture_;

        mutable std::mutex mutex_;
    };

} // namespace cxx
