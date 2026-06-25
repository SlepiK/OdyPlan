#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDSOURCEOPERATOR_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDSOURCEOPERATOR_H

#include "core/operator/flow/emitter.h"
#include "core/port/port.h"
#include "source_operator.h"

namespace sierra::odyplan::v1::core::opr {
    template <port::PortType TOut>
    class TypedSourceOperator : public ISourceOperator, public flow::IEmitting<TOut> {
    public:
        using OperatorInput = port::NullPort;
        using OperatorOutput = TOut;

        void setEmitter(flow::Emitter<TOut>* emitter) final {
            mEmitter = emitter;
        }

    protected:
        void emit(const TOut& tuple) {
            if (mEmitter != nullptr) {
                mEmitter->emit(tuple);
            }
        }

    private:
        flow::Emitter<TOut>* mEmitter = nullptr;
    };
}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDSOURCEOPERATOR_H