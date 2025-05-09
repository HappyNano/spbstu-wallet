#pragma once

#include <imgui.h>

#include <optional>

namespace cxx {
    class Context {
    public:
        virtual auto getStatusBarHeight() noexcept -> std::optional< int >;
        virtual auto getBackgroundColor() noexcept -> std::optional< ImVec4 >;

    protected:
        std::optional< int > statusBarHeight_;
        std::optional< ImVec4 > backgroudColor_;
    };
} // namespace cxx
