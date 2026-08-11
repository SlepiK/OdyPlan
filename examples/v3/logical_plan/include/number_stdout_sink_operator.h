#ifndef SIERRA_ODYPLAN_V3_EXAMPLE_LOGICALPLAN_NUMBERSTDOUTSINKOPERATOR_HPP
#define SIERRA_ODYPLAN_V3_EXAMPLE_LOGICALPLAN_NUMBERSTDOUTSINKOPERATOR_HPP

#include <core/logical/operators/sink/typed_sink_operator.h>

namespace sierra::odyplan::v3::example::logical_plan {
    class NumberStdoutSinkOperator final
        : public core::logical::operators::sink::TypedSinkOperator {
    public:
        static constexpr core::shared::types::OperatorTypeId TYPE_ID = 1;

        NumberStdoutSinkOperator() = default;

        [[nodiscard]] core::shared::types::OperatorTypeId getOperatorTypeId() const override {
            return TYPE_ID;
        }

    protected:
        void validateInput() override {}
    };

    class DoubleNumberStdoutSinkOperator final
        : public core::logical::operators::sink::TypedSinkOperator {
    public:
        static constexpr core::shared::types::OperatorTypeId TYPE_ID = 1;

        DoubleNumberStdoutSinkOperator() = default;

        [[nodiscard]] core::shared::types::OperatorTypeId getOperatorTypeId() const override {
            return TYPE_ID;
        }

    protected:
        void validateInput() override {}
    };
}  // namespace sierra::odyplan::v3::example::logical_plan

#endif  // SIERRA_ODYPLAN_V3_EXAMPLE_LOGICALPLAN_NUMBERSTDOUTSINKOPERATOR_HPP