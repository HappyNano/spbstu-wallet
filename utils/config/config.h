#pragma once

#include <utils/config/interfaces/i_config.h>

namespace util {

    class Config final: public IConfig {
    public:
        Config();

    private:
        nlohmann::json config_;
    };

} // namespace util
