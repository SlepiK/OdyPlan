#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDSOURCEOPERATOR_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDSOURCEOPERATOR_H

#include "core/operator/flow/emitter.h"
#include "core/tuple/tuple.h"
#include "source_operator.h"

namespace sierra::odyplan::v1::core::opr {
    template <tuple::Tuple TOut>
    class TypedSourceOperator : public ISourceOperator {
    public:
        virtual void produce(flow::Emitter<TOut>* out) = 0;
    };
}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDSOURCEOPERATOR_H