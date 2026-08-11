#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_PIPE_TYPED_PIPE_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_PIPE_TYPED_PIPE_OPERATOR_HPP

#include <core/physical/operators/pipe/pipe_operator.h>

namespace sierra::odyplan::v3::core::physical::operators::pipe {
    class TypedPipeOperator : public IPipeOperator {
    public:
        explicit TypedPipeOperator(port::Ports inputPorts, port::Ports outputPorts);
    };
}  // namespace sierra::odyplan::v3::core::physical::operators::pipe

#endif  // SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_PIPE_TYPED_PIPE_OPERATOR_HPP