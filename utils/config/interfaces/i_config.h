#pragma once

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

namespace util {
    class IConfig {
    public:
        IConfig() = default;

        virtual ~IConfig() = default;
    };
} // namespace util
