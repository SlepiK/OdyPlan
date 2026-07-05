#ifndef SIERRA_ODYPLAN_V2_EXAMPLE_LOGICALPLAN_INTPIPEOPERATOR_H
#define SIERRA_ODYPLAN_V2_EXAMPLE_LOGICALPLAN_INTPIPEOPERATOR_H

#include "core/logical/operator/pipe/pipe_operator.h"
#include "const_number_source_operator.h"

namespace sierra::odyplan::v2::example::logical_plan {
    class IntPipeOperator final : public core::logical::opr::TypedPipeOperator {
    public:
        static constexpr sierra::odyplan::v2::core::common::identifier::OperatorTypeId TYPE_ID = 2;

        IntPipeOperator() : core::logical::opr::TypedPipeOperator(
                sierra::odyplan::v2::core::logical::port::Ports {sierra::odyplan::v2::example::logical_plan::createIntPort()}
            ) {
        }

        [[nodiscard]] sierra::odyplan::v2::core::common::identifier::OperatorTypeId getTypeId() const override {
            return TYPE_ID;
        }

    protected:
        void validate() override {
        }
    };
} // namespace sierra::odyplan::v2::example::logical_plan

#endif //SIERRA_ODYPLAN_V2_EXAMPLE_LOGICALPLAN_INTPIPEOPERATOR_H