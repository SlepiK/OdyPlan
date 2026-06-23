#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_FLOW_EMITTER_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_FLOW_EMITTER_H

#include "core/tuple/tuple.h"

namespace sierra::odyplan::v1::core::opr::flow {
    template <tuple::Tuple TOut>
    class Emitter {
    public:
        Emitter() = default;
        virtual ~Emitter() = default;

        Emitter(const Emitter&) = delete;
        Emitter& operator=(const Emitter&) = delete;

        Emitter(Emitter&&) = delete;
        Emitter& operator=(Emitter&&) = delete;

        virtual void emit(const TOut& tuple) = 0;
    };
}  // namespace sierra::odyplan::v1::core::opr::flow

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_FLOW_EMITTER_H