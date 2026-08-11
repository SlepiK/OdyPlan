#ifndef SIERRA_ODYPLAN_V3_BASIC_WINDOW_WINDOW_RANGE_HPP
#define SIERRA_ODYPLAN_V3_BASIC_WINDOW_WINDOW_RANGE_HPP
#include <cstdint>

namespace sierra::odyplan::v3::basic::window {
    struct WindowRange {
        std::uint64_t start;
        std::uint64_t count;
    };
}  // namespace sierra::odyplan::v3::basic::window

#endif  // SIERRA_ODYPLAN_V3_BASIC_WINDOW_WINDOW_RANGE_HPP