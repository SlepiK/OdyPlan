#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_SINK_SINKOPERATOR_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_SINK_SINKOPERATOR_H

#include "v2/core/logical/operator/operator.h"
#include "v2/core/logical/operator/operator_trait.h"
#include "v2/core/logical/port/port.h"

namespace sierra::odyplan::v2::core::logical::opr {
    class ISinkOperator : public IOperator {
    };

    template <port::PortType In>
    class TypedSinkOperator : public ISinkOperator {
    };

    template <port::PortType In>
    struct OperatorTraits<TypedSinkOperator<In>> {
        using Inputs = port::Ports<In>;
        using Outputs = port::NullPort;
    };
}  // namespace sierra::odyplan::v2::core::logical::opr

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_SINK_SINKOPERATOR_H