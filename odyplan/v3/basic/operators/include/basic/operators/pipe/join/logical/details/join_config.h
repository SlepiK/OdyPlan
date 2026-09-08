#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_LOGICAL_DETAILS_JOIN_CONFIG_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_LOGICAL_DETAILS_JOIN_CONFIG_HPP

#include <cstdint>

namespace sierra::odyplan::v3::basic::operators::pipe::join::logical::details {
    struct JoinConfig {
        std::int64_t allowedLateness{0};
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::logical::details

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_LOGICAL_DETAILS_JOIN_CONFIG_HPP