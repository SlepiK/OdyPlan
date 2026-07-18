#ifndef SIERRA_ODYPLAN_V2_COMMON_WINDOW_WINDOWRANGE_H
#define SIERRA_ODYPLAN_V2_COMMON_WINDOW_WINDOWRANGE_H
#include <cstdint>

namespace sierra::odyplan::v2::common::window {
    struct WindowRange {
        std::uint64_t start;
        std::uint64_t count;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_WINDOW_WINDOWRANGE_H