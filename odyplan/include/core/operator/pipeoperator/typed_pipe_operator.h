#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDPIPEOPERATOR_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDPIPEOPERATOR_H

#include "core/operator/flow/emitter.h"
#include "core/operator/ingest/ingest.h"
#include "core/port/port.h"
#include "pipe_operator.h"

namespace sierra::odyplan::v1::core::opr {
    template <port::PortsType TIn, port::PortType TOut>
    class TypedPipeOperator : public IPipeOperator,
                              public ingest::IngestPorts<TIn>,
                              public flow::IEmitting<TOut> {
    public:
        using Input = TIn;
        using Output = TOut;

        static constexpr std::size_t INPUT_PORT_COUNT = TIn::SIZE;

        void setEmitter(flow::Emitter<TOut>* emitter) final {
            mEmitter = emitter;
        }

    protected:
        void emit(const TOut::Fields& tuple) {
            if (mEmitter != nullptr) {
                mEmitter->emit(tuple);
            }
        }
    private:
        flow::Emitter<TOut>* mEmitter = nullptr;
    };
}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDPIPEOPERATOR_H