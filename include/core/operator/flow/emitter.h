#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_FLOW_EMITTER_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_FLOW_EMITTER_H

#include "core/tuple/tuple.h"

namespace sierra::odyplan::v1::core::opr::flow {
    template <tuple::Tuple TOut>
    class IEmitter {
    public:
        virtual void emit(const TOut& value) = 0;
    };
}  // namespace sierra::odyplan::v1::core::opr::flow

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_FLOW_EMITTER_H