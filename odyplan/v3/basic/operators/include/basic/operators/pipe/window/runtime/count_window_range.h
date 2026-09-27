#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_RUNTIME_COUNT_WINDOW_RANGE_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_RUNTIME_COUNT_WINDOW_RANGE_HPP
#include <cstdint>

namespace sierra::odyplan::v3::basic::operators::pipe::window::runtime {
    struct CountWindowRange {
        std::uint64_t start;
        std::uint64_t count;
    };
}

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_RUNTIME_COUNT_WINDOW_RANGE_HPP