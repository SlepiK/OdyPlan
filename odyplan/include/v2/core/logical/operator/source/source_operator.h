#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_SOURCE_SOURCEOPERATOR_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_SOURCE_SOURCEOPERATOR_H

#include "v2/core/logical/operator/operator.h"
#include "v2/core/logical/operator/operator_trait.h"
#include "v2/core/logical/port/port.h"

namespace sierra::odyplan::v2::core::logical::opr {
    class ISourceOperator : public IOperator {
    };

    template <port::PortType Out>
    class TypedSourceOperator : public ISourceOperator {
    };

    template <port::PortType Out>
    struct OperatorTraits<TypedSourceOperator<Out>> {
        using Inputs = port::NullPort;
        using Outputs = port::Ports<Out>;
    };

}  // namespace sierra::odyplan::v2::core::logical::opr

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_SOURCE_SOURCEOPERATOR_H