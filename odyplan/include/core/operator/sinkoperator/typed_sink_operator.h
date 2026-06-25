#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDSINKOPERATOR_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDSINKOPERATOR_H

#include "core/operator/ingest/ingest.h"
#include "core/port/port.h"
#include "sink_operator.h"

namespace sierra::odyplan::v1::core::opr {
    template <port::PortType TIn>
    class TypedSinkOperator : public ISinkOperator, public ingest::IIngestPort<0, TIn> {
    public:
        using OperatorInput = TIn;
        using OperatorOutput = port::NullPort;
    };
}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDSINKOPERATOR_H