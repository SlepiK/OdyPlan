#ifndef SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_PIPES_TYPED_PIPE_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_PIPES_TYPED_PIPE_OPERATOR_HPP

#include <core/logical/operators/pipes/pipe_operator.h>

namespace sierra::odyplan::v3::core::logical::operators::pipes {
    class TypedPipeOperator : public IPipeOperator {
    public:
        explicit TypedPipeOperator() = default;
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_PIPES_TYPED_PIPE_OPERATOR_HPP