#ifndef SIERRA_ODYPLAN_V2_EXAMPLE_LOGICALPLAN_NUMBERSTDOUTSINKOPERATOR_H
#define SIERRA_ODYPLAN_V2_EXAMPLE_LOGICALPLAN_NUMBERSTDOUTSINKOPERATOR_H

#include "core/logical/operator/sink/sink_operator.h"
#include "const_number_source_operator.h"

namespace sierra::odyplan::v2::example::logical_plan {
    class NumberStdoutSinkOperator final : public v2::core::logical::opr::TypedSinkOperator {
    public:
        static constexpr sierra::odyplan::v2::core::common::identifier::OperatorTypeId TYPE_ID = 1;

        NumberStdoutSinkOperator() : TypedSinkOperator(createIntPort()) {
        }

        [[nodiscard]] sierra::odyplan::v2::core::common::identifier::OperatorTypeId getTypeId() const override {
            return TYPE_ID;
        }
    };

    class DoubleNumberStdoutSinkOperator final : public v2::core::logical::opr::TypedSinkOperator {
    public:
        static constexpr sierra::odyplan::v2::core::common::identifier::OperatorTypeId TYPE_ID = 1;

        DoubleNumberStdoutSinkOperator() : TypedSinkOperator(createDoublePort()) {
        }

        [[nodiscard]] sierra::odyplan::v2::core::common::identifier::OperatorTypeId getTypeId() const override {
            return TYPE_ID;
        }
    };
} // namespace sierra::odyplan::v2::example::logical_plan

#endif //SIERRA_ODYPLAN_V2_EXAMPLE_LOGICALPLAN_NUMBERSTDOUTSINKOPERATOR_H