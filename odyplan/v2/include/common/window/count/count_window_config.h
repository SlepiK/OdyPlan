#ifndef SIERRA_ODYPLAN_V2_COMMON_WINDOW_COUNT_WINDOWCONFIG_H
#define SIERRA_ODYPLAN_V2_COMMON_WINDOW_COUNT_WINDOWCONFIG_H

#include <chrono>
#include <common/window/time/time_window_config.h>

namespace sierra::odyplan::v2::common::window::count {
    struct CountWindowConfig {
        WindowType type;
        std::uint64_t size;
        std::uint64_t slide;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_WINDOW_COUNT_WINDOWCONFIG_H