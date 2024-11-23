#pragma once

#include <string>

namespace fh {

struct Config {
    int         version          = 1;
    std::string i18n_location    = "zh_CN";
    struct {
        bool tick_command = true;
        bool enable_microtick = false;
    } function{};
};

} // namespace fh