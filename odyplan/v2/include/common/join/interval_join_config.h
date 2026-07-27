#ifndef SIERRA_ODYPLAN_V2_COMMON_JOIN_INTERVAL_JOIN_CONFIG_H
#define SIERRA_ODYPLAN_V2_COMMON_JOIN_INTERVAL_JOIN_CONFIG_H
#include <chrono>
#include <cstddef>

namespace sierra::odyplan::v2::common::join {
    struct IntervalJoinConfig {
        std::size_t leftTimeStampIdx;
        std::size_t rightTimeStampIdx;
        std::chrono::milliseconds leftWindowSize;
        std::chrono::milliseconds rightWindowSize;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_JOIN_INTERVAL_JOIN_CONFIG_H