#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDSINKOPERATOR_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDSINKOPERATOR_H

#include "core/tuple/tuple.h"
#include "sink_operator.h"

namespace sierra::odyplan::v1::core::opr {
    template <tuple::Tuple TIn>
    class TypedSinkOperator : public ISinkOperator {
    public:
        virtual void consume(const TIn& tuple) = 0;
    };
}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDSINKOPERATOR_H