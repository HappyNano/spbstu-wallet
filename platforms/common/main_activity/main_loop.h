#pragma once

#include <memory>

#include "context.h"

namespace cxx {
    /**
     * @brief MainLoop class
     */
    class MainLoop final {
    public:
        MainLoop() = default;
        ~MainLoop() = default;

        void draw(const std::shared_ptr< Context > & context);
    };
} // namespace cxx
