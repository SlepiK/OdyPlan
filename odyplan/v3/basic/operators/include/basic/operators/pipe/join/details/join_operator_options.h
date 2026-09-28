#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_DETAILS_JOIN_OPERATOR_OPTIONS_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_DETAILS_JOIN_OPERATOR_OPTIONS_HPP

#include <basic/operators/pipe/join/details/join_operator_card.h>
#include <core/shared/types/global_types.h>

namespace sierra::odyplan::v3::basic::operators::pipe::join::details {
    struct JoinOperatorOptions {
        core::shared::types::Timestamp allowedLateness{0};
        JoinOperatorCard card = JoinOperatorCard::MANY_MANY;
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::details

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_DETAILS_JOIN_OPERATOR_OPTIONS_HPP