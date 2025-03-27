#pragma once

#include <imgui.h>

#include <optional>

namespace cxx {
    struct Context {
        // void setParam(std::string key, );
        std::optional<int> statusBarHeight;
        ImVec4 backgroudColor;
    };
} // namespace cxx
