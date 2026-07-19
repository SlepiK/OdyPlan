#ifndef SIERRA_ODYPLAN_V2_COMMON_WINDOW_TIME_WINDOWCONFIG_H
#define SIERRA_ODYPLAN_V2_COMMON_WINDOW_TIME_WINDOWCONFIG_H
#include <chrono>
#include <common/window/window_type.h>

namespace sierra::odyplan::v2::common::window::time {
    struct TimeWindowConfig {
        WindowType type;
        std::chrono::milliseconds size;
        std::chrono::milliseconds slide;
        std::size_t timestampIndex;
        std::chrono::milliseconds allowedLateness{0};
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_WINDOW_TIME_WINDOWCONFIG_H