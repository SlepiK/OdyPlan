#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_JOIN_OPERATOR_PHYSICAL_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_JOIN_OPERATOR_PHYSICAL_H
#include "core/physical/operator/pipe/pipe_operator.h"

namespace sierra::odyplan::v2::common::opr::pipe::physical {
    class JoinOperator : public core::physical::opr::TypedPipeOperator {
    public:

        JoinOperator(core::physical::port::Port inputPort, core::physical::port::Port outputPort)
            : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)},
                                core::physical::port::Ports{std::move(outputPort)}) {
        }

        void consume(core::physical::segment::SegmentContext &segmentCtx,
                     core::common::identifier::PortId /*inputPort*/, const core::physical::port::PortTuple &tuple,
                     const core::physical::port::PortTupleConsumers &consumers) override {
        }

        void end(core::physical::segment::SegmentContext &segmentCtx,
                 const core::common::identifier::PortId /*inputPort*/,
                 const core::physical::port::PortTupleConsumers & /*consumers*/) override {
        }

        void declareState(core::physical::state::StateRegistry &stateRegistry, llvm::Module &module,
                          llvm::LLVMContext &context) override {
        }
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_JOIN_OPERATOR_PHYSICAL_H