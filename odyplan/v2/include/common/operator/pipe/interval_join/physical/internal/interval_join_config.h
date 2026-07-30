#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_INTERVAL_JOIN_OPERATOR_PHYSICAL_INTERNAL_CONFIG_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_INTERVAL_JOIN_OPERATOR_PHYSICAL_INTERNAL_CONFIG_H
#include <chrono>
#include <cstddef>

namespace sierra::odyplan::v2::common::opr::pipe::physical::internal {
    struct IntervalJoinConfig {
        std::chrono::milliseconds leftWindowSize;
        std::chrono::milliseconds rightWindowSize;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_INTERVAL_JOIN_OPERATOR_PHYSICAL_INTERNAL_CONFIG_H