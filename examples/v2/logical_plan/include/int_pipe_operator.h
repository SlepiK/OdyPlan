#ifndef SIERRA_ODYPLAN_V2_EXAMPLE_LOGICALPLAN_INTPIPEOPERATOR_H
#define SIERRA_ODYPLAN_V2_EXAMPLE_LOGICALPLAN_INTPIPEOPERATOR_H

#include "core/logical/operator/pipe/pipe_operator.h"

namespace sierra::odyplan::v2::example::logical_plan {
    class IntPipeOperator final : public core::logical::opr::TypedPipeOperator {
    public:
        static constexpr core::common::identifier::OperatorTypeId TYPE_ID = 2;

        IntPipeOperator() = default;

        [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
            return TYPE_ID;
        }

    protected:
        void validateInput() override {
        }
    };
} // namespace sierra::odyplan::v2::example::logical_plan

#endif //SIERRA_ODYPLAN_V2_EXAMPLE_LOGICALPLAN_INTPIPEOPERATOR_H