#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDFUSEDVIRTUALOPERATOR_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDFUSEDVIRTUALOPERATOR_H

#include "core/operator/pipeoperator/typed_pipe_operator.h"
#include "core/operator/sinkoperator/typed_sink_operator.h"
#include "core/operator/sourceoperator/typed_source_operator.h"
#include "core/port/port.h"
#include "virtual_operator.h"

namespace sierra::odyplan::v1::core::opr {
    template <port::PortsType TIn, port::PortType TOut>
    class TypedVirtualOperator : public IVirtualOperator, public TypedPipeOperator<TIn, TOut> {};
}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDFUSEDVIRTUALOPERATOR_H