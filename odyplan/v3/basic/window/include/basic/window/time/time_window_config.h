#ifndef SIERRA_ODYPLAN_V3_BASIC_WINDOW_TIME_TIME_WINDOW_CONFIG_HPP
#define SIERRA_ODYPLAN_V3_BASIC_WINDOW_TIME_TIME_WINDOW_CONFIG_HPP

#include <cstdint>
#include <optional>

namespace sierra::odyplan::v3::basic::window::time {
    struct TimeWindowConfig {
        std::uint64_t size = 1;
        std::optional<std::uint64_t> slide = std::nullopt;
        std::optional<std::uint64_t> advance = std::nullopt;
    };
}  // namespace sierra::odyplan::v3::basic::window::time

#endif  // SIERRA_ODYPLAN_V3_BASIC_WINDOW_TIME_TIME_WINDOW_CONFIG_HPP