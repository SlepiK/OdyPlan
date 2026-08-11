#ifndef SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_PIPE_TYPED_PIPE_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_PIPE_TYPED_PIPE_OPERATOR_HPP

#include <core/logical/operators/pipe/pipe_operator.h>

namespace sierra::odyplan::v3::core::logical::operators::pipe {
    class TypedPipeOperator : public IPipeOperator {
    public:
        explicit TypedPipeOperator() = default;
    };
}  // namespace sierra::odyplan::v3::core::logical::operators::pipe

#endif  // SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_PIPE_TYPED_PIPE_OPERATOR_HPP