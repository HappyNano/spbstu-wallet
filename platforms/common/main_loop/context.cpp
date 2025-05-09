#include "context.h"

using namespace cxx;

auto Context::getStatusBarHeight() noexcept -> std::optional< int > {
    return statusBarHeight_;
}

auto Context::getBackgroundColor() noexcept -> std::optional< ImVec4 > {
    return backgroudColor_;
}
