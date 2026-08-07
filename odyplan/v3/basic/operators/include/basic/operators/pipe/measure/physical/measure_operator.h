#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MEASURE_PHYSICAL_MEASURE_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MEASURE_PHYSICAL_MEASURE_OPERATOR_HPP

#include <core/physical/operators/pipe/typed_pipe_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::measure::physical {
    class MeasureOperator : public core::physical::operators::pipe::TypedPipeOperator {
    public:
        using Mapping = std::vector<std::size_t>;

        MeasureOperator(core::physical::port::Port inputPort, core::physical::port::Port outputPort, std::string label);

        void consume(core::physical::segment::SegmentContext &segmentCtx,
                     core::shared::identifiers::PortId /*inputPort*/, const core::physical::data::DataPackage &tuple,
                     const core::physical::segment::SegmentDataConsumers &consumers) override;

        void end(core::physical::segment::SegmentContext &segmentCtx,
                 core::shared::identifiers::PortId /*inputPort*/,
                 const core::physical::segment::SegmentDataConsumers & /*consumers*/) override;

        void declareState(core::physical::state::StateRegistry &stateRegistry, llvm::Module &module,
                          llvm::LLVMContext &context) override;

    private:
        std::string mLabel;
        core::physical::state::StateHandle mStateHandle;

        llvm::Value *loadStatePtr(core::physical::segment::SegmentContext &segmentCtx) const;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MEASURE_PHYSICAL_MEASURE_OPERATOR_HPP