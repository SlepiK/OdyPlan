#ifndef SIERRA_ODYPLAN_V3_EXAMPLE_LOGICALPLAN_INTPIPEOPERATOR_HPP
#define SIERRA_ODYPLAN_V3_EXAMPLE_LOGICALPLAN_INTPIPEOPERATOR_HPP

#include <core/logical/operators/pipe/typed_pipe_operator.h>

namespace sierra::odyplan::v3::example::logical_plan {
    class IntPipeOperator final : public core::logical::operators::pipe::TypedPipeOperator {
    public:
        static constexpr core::shared::types::OperatorTypeId TYPE_ID = 2;

        IntPipeOperator() = default;

        [[nodiscard]] core::shared::types::OperatorTypeId getOperatorTypeId() const override {
            return TYPE_ID;
        }

    protected:
        void validateInput() override {
        }
    };
}

#endif //SIERRA_ODYPLAN_V3_EXAMPLE_LOGICALPLAN_INTPIPEOPERATOR_HPP