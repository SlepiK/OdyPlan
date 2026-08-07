#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_LOGICAL_DETAILS_JOIN_CONFIG_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_LOGICAL_DETAILS_JOIN_CONFIG_HPP
#include <chrono>
#include <cstddef>

namespace sierra::odyplan::v3::basic::operators::pipe::join::logical::details {
    struct JoinConfig {
        std::chrono::milliseconds leftWindowSize;
        std::chrono::milliseconds rightWindowSize;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_LOGICAL_DETAILS_JOIN_CONFIG_HPP