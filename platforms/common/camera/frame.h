#pragma once

#include <memory>

namespace cxx {
    struct Frame final {
        void rotate();
        auto isVertical() const -> bool;

    public:
        const std::unique_ptr< int8_t[] > data;
        int width;
        int height;
        const int channels;
    };
} // namespace cxx
