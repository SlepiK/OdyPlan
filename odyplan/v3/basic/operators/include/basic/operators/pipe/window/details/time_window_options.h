#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_DETAILS_TIME_WINDOW_OPTIONS_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_DETAILS_TIME_WINDOW_OPTIONS_HPP

#include <cstdint>
#include <optional>

namespace sierra::odyplan::v3::basic::operators::pipe::window::details {
    struct TimeWindowOptions {
        std::size_t size = 1;
        std::optional<std::size_t> slide = std::nullopt;
        std::optional<std::size_t> advance = std::nullopt;
    };
}  // namespace sierra::odyplan::v3::basic::window::time

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_DETAILS_TIME_WINDOW_OPTIONS_HPP