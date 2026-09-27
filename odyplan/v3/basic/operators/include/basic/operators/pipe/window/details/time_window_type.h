#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_DETAILS_TIME_WINDOW_TYPE_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_DETAILS_TIME_WINDOW_TYPE_HPP

#include <cstdint>

namespace sierra::odyplan::v3::basic::operators::pipe::window::details {
    enum class TimeWindowType : std::uint8_t {
        SLIDING,
        ADVANCE,
        PERIODIC
    };
}

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_DETAILS_TIME_WINDOW_TYPE_HPP