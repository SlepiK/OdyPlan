#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_FLOW_DRAINAIBLE_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_FLOW_DRAINAIBLE_H

#include "core/tuple/tuple.h"
#include "emitter.h"

namespace sierra::odyplan::v1::core::opr::flow {
    template <port::PortType TOut>
    class IDrainable {
    public:
        IDrainable() = default;
        virtual ~IDrainable() = default;

        IDrainable(const IDrainable&) = delete;
        IDrainable& operator=(const IDrainable&) = delete;

        IDrainable(IDrainable&&) = delete;
        IDrainable& operator=(IDrainable&&) = delete;

        virtual bool drain(Emitter<TOut>& out) = 0;
        [[nodiscard]] virtual bool isReadyToDrain() const = 0;
    };
}  // namespace sierra::odyplan::v1::core::opr::flow

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_FLOW_DRAINAIBLE_H