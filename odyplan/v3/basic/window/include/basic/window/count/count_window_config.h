#ifndef SIERRA_ODYPLAN_V2_COMMON_WINDOW_COUNT_WINDOWCONFIG_H
#define SIERRA_ODYPLAN_V2_COMMON_WINDOW_COUNT_WINDOWCONFIG_H

#include <basic/window/time/time_window_config.h>

namespace sierra::odyplan::v3::basic::window::count {
    struct CountWindowConfig {
        WindowType type;
        std::uint64_t size;
        std::uint64_t slide;
        std::uint64_t advance;
    };
}  // namespace sierra::odyplan::v3::basic::window::count

#endif  // SIERRA_ODYPLAN_V2_COMMON_WINDOW_COUNT_WINDOWCONFIG_H