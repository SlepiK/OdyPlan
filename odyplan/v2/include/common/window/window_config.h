#ifndef SIERRA_ODYPLAN_V2_COMMON_WINDOW_WINDOWCONFIG_H
#define SIERRA_ODYPLAN_V2_COMMON_WINDOW_WINDOWCONFIG_H
#include "common/operator/pipe/window_operator.h"

namespace sierra::odyplan::v2::common::window {
    struct WindowConfig {
        opr::pipe::logical::WindowOperator::WindowType type;
        opr::pipe::logical::WindowOperator::WindowTrigger trigger;
        std::uint64_t size;
        std::uint64_t slide;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_WINDOW_WINDOWCONFIG_H