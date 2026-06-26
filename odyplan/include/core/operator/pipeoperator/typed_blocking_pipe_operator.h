#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDBLOCKINGPIPEOPERATOR_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDBLOCKINGPIPEOPERATOR_H

#include "core/operator/flow/drainable.h"
#include "core/operator/ingest/ingest.h"
#include "core/port/port.h"
#include "pipe_operator.h"

namespace sierra::odyplan::v1::core::opr {
    template <port::PortsType TIn, port::PortType TOut>
    class TypedBlockingPipeOperator : public IPipeOperator,
                                      public ingest::IngestPorts<TIn>,
                                      public flow::IDrainable<TOut> {
    public:
        using Input = TIn;
        using Output = TOut;

        static constexpr std::size_t INPUT_PORT_COUNT = TIn::SIZE;

        [[nodiscard]] bool isReadyToDrain() const override {
            return true;
        }
    };
}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_TYPEDBLOCKINGPIPEOPERATOR_H