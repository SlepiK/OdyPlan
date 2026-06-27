#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_PIPE_PIPEOPERATOR_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_PIPE_PIPEOPERATOR_H

#include "v2/core/logical/operator/operator.h"
#include "v2/core/logical/operator/operator_trait.h"
#include "v2/core/logical/port/port.h"

namespace sierra::odyplan::v2::core::logical::opr {
    class IPipeOperator : public IOperator {
    };

    template <port::PortsType In, port::PortType Out>
    class TypedPipeOperator : public IPipeOperator {
    };

    template <port::PortsType In, port::PortType Out>
    struct OperatorTraits<TypedPipeOperator<In, Out>> {
        using Inputs = In;
        using Outputs = port::Port<Out>;
    };


}  // namespace sierra::odyplan::v2::core::logical::opr

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_PIPE_PIPEOPERATOR_H