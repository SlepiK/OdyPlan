#ifndef SIERRA_ODYPLAN_V2_EXAMPLE_LOGICALPLAN_NUMBERSTDOUTSINKOPERATOR_H
#define SIERRA_ODYPLAN_V2_EXAMPLE_LOGICALPLAN_NUMBERSTDOUTSINKOPERATOR_H

#include "core/logical/operator/sink/sink_operator.h"

namespace sierra::odyplan::v2::example::logical_plan {
    class NumberStdoutSinkOperator final : public core::logical::opr::TypedSinkOperator {
    public:
        static constexpr core::common::identifier::OperatorTypeId TYPE_ID = 1;

        NumberStdoutSinkOperator() = default;

        [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
            return TYPE_ID;
        }

    protected:
        void validateInput() override {
        }
    };

    class DoubleNumberStdoutSinkOperator final : public core::logical::opr::TypedSinkOperator {
    public:
        static constexpr core::common::identifier::OperatorTypeId TYPE_ID = 1;

        DoubleNumberStdoutSinkOperator() = default;

        [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
            return TYPE_ID;
        }

    protected:
        void validateInput() override {
        }
    };
} // namespace sierra::odyplan::v2::example::logical_plan

#endif //SIERRA_ODYPLAN_V2_EXAMPLE_LOGICALPLAN_NUMBERSTDOUTSINKOPERATOR_H