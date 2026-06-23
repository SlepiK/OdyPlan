#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDBLOCKINGPIPEOPERATOR_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDBLOCKINGPIPEOPERATOR_H

#include "core/operator/flow/drainable.h"
#include "core/operator/sinkoperator/typed_sink_operator.h"
#include "core/operator/sourceoperator/typed_source_operator.h"
#include "core/tuple/tuple.h"
#include "pipe_operator.h"

namespace sierra::odyplan::v1::core::opr {
    template <tuple::Tuple TOut, tuple::Tuple... TIn>
    class TypedBlockingPipeOperator : public IPipeOperator,
                                      public ingest::IngestPorts<TIn...>,
                                      public flow::IDrainable<TOut> {
    public:
        static constexpr std::size_t INPUT_PORT_COUNT = sizeof...(TIn);

        [[nodiscard]] bool isReadyToDrain() const override {
            return true;
        }
    };
}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDBLOCKINGPIPEOPERATOR_H