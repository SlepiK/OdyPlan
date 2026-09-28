#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_DETAILS_JOIN_OPERATOR_CARD_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_DETAILS_JOIN_OPERATOR_CARD_HPP

#include <cstdint>

namespace sierra::odyplan::v3::basic::operators::pipe::join::details {
    enum class JoinOperatorCard : uint8_t {
        MANY_MANY = 0,
        ONE_MANY = 1,
        MANY_ONE = 2,
        ONE_ONE = 3
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::details

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_DETAILS_JOIN_OPERATOR_CARD_HPP