#ifndef SIERRA_ODYPLAN_V2_COMMON_WINDOW_WINDOWCONFIG_H
#define SIERRA_ODYPLAN_V2_COMMON_WINDOW_WINDOWCONFIG_H
#include <chrono>
#include <variant>

#include "common/operator/pipe/window_operator.h"

namespace sierra::odyplan::v2::common::window {
    struct CountWindowConfig {
        opr::pipe::logical::WindowOperator::WindowType type;
        std::uint64_t size;
        std::uint64_t slide;
    };

    struct TimeWindowConfig {
        opr::pipe::logical::WindowOperator::WindowType type;
        std::chrono::milliseconds size;
        std::chrono::milliseconds slide;
    };

    using WindowConfig = std::variant<CountWindowConfig, TimeWindowConfig>;
}

#endif //SIERRA_ODYPLAN_V2_COMMON_WINDOW_WINDOWCONFIG_H