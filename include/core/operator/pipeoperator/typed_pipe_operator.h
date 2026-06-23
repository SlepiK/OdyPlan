#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDPIPEOPERATOR_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDPIPEOPERATOR_H

#include "core/operator/sinkoperator/typed_sink_operator.h"
#include "core/operator/sourceoperator/typed_source_operator.h"
#include "core/tuple/tuple.h"
#include "pipe_operator.h"

namespace sierra::odyplan::v1::core::opr {
    template <tuple::Tuple TIn, tuple::Tuple TOut>
    class TypedPipeOperator : public IPipeOperator {
    public:
        virtual void consume(const TIn& tupleIn, flow::Emitter<TOut>* out) = 0;
    };
}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDPIPEOPERATOR_H