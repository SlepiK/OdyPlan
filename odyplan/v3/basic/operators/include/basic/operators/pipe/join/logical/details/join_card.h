#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_LOGICAL_DETAILS_JOIN_CARD_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_LOGICAL_DETAILS_JOIN_CARD_HPP

#include <cstdint>
#include <iostream>

namespace sierra::odyplan::v3::basic::operators::pipe::join::logical::details {
    enum class JoinCard : uint8_t {
        MANY_MANY = 0,
        ONE_MANY = 1,
        MANY_ONE = 2,
        ONE_ONE = 3
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::logical::details

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_LOGICAL_DETAILS_JOIN_CARD_HPP