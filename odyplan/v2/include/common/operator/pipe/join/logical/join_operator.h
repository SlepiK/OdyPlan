#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_JOIN_OPERATOR_LOGICAL_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_JOIN_OPERATOR_LOGICAL_H

#include <utility>
#include "core/logical/operator/pipe/pipe_operator.h"

namespace sierra::odyplan::v2::common::opr::pipe::logical {
    class JoinOperator : public core::logical::opr::TypedPipeOperator {
    public:
        static constexpr core::common::identifier::OperatorTypeId TYPE_ID = std::to_underlying(
            core::common::identifier::Operator::JOIN);

        explicit JoinOperator() = default;

        [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
            return TYPE_ID;
        }

    protected:
        void validate() override {
        }
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_JOIN_OPERATOR_LOGICAL_H