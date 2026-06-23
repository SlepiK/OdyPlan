#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDPIPEOPERATOR_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDPIPEOPERATOR_H

#include "core/operator/sinkoperator/typed_sink_operator.h"
#include "core/operator/sourceoperator/typed_source_operator.h"
#include "core/tuple/tuple.h"
#include "pipe_operator.h"

namespace sierra::odyplan::v1::core::opr {
    template <tuple::Tuple TOut, tuple::Tuple... TIn>
    class TypedPipeOperator : public IPipeOperator,
                              public ingest::IngestPorts<TIn...>,
                              public flow::IEmitting<TOut> {
    public:
        static constexpr std::size_t INPUT_PORT_COUNT = sizeof...(TIn);

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

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDPIPEOPERATOR_H