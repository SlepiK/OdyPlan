#ifndef SIERRA_ODYPLAN_V2_EXAMPLE_LOGICALPLAN_INTPIPEOPERATOR_H
#define SIERRA_ODYPLAN_V2_EXAMPLE_LOGICALPLAN_INTPIPEOPERATOR_H

#include "core/logical/operator/pipe/pipe_operator.h"
#include "const_number_source_operator.h"

namespace sierra::odyplan::v2::example::logical_plan {
    class IntPipeOperator final : public core::logical::opr::TypedPipeOperator {
    public:
        static constexpr sierra::odyplan::v2::core::common::type::TypeId TYPE_ID = 2;

        IntPipeOperator() = default;

        [[nodiscard]] sierra::odyplan::v2::core::common::type::TypeId getTypeId() const override {
            return TYPE_ID;
        }

    protected:
        void validate() override {
        }
    };
} // namespace sierra::odyplan::v2::example::logical_plan

#endif //SIERRA_ODYPLAN_V2_EXAMPLE_LOGICALPLAN_INTPIPEOPERATOR_H