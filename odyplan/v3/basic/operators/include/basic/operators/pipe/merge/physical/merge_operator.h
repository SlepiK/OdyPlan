#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MERGE_PHYSICAL_MERGE_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MERGE_PHYSICAL_MERGE_OPERATOR_HPP

#include <core/physical/operators/pipe/typed_pipe_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::merge::physical {
    class MergeOperator : public core::physical::operators::pipe::TypedPipeOperator {
    public:
        MergeOperator(core::physical::port::Port inputPort, core::physical::port::Port outputPort);

        void declareState(core::physical::state::StateRegistry &stateRegistry, llvm::Module & /*module*/,
                          llvm::LLVMContext & /*context*/) override;

        void consume(core::physical::segment::SegmentContext &segmentCtx,
                     core::shared::identifiers::PortId /*inputPort*/,
                     const core::physical::data::DataPackage &tuple,
                     const core::physical::segment::SegmentDataConsumers &consumers) override;

    private:
        core::physical::state::StateHandle mStateHandle;

        llvm::Value *loadStatePtr(core::physical::segment::SegmentContext &segmentCtx) const;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MERGE_PHYSICAL_MERGE_OPERATOR_HPP