#ifndef SIERRA_ODYPLAN_V3_BASIC_WINDOW_TIME_TIME_WINDOW_CONFIG_HPP
#define SIERRA_ODYPLAN_V3_BASIC_WINDOW_TIME_TIME_WINDOW_CONFIG_HPP

#include <chrono>
#include <basic/window/window_type.h>

namespace sierra::odyplan::v3::basic::window::time {
    struct TimeWindowConfig {
        WindowType type;
        std::chrono::milliseconds size;
        std::chrono::milliseconds slide;
        std::size_t timestampIndex;
        std::chrono::milliseconds allowedLateness{0};
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_WINDOW_TIME_TIME_WINDOW_CONFIG_HPP