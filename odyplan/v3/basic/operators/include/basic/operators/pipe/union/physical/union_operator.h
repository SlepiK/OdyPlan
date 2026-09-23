#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_PHYSICAL_UNION_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_PHYSICAL_UNION_OPERATOR_HPP

#include <core/physical/operators/pipe/typed_pipe_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::union_::physical {// NOLINT(readability-identifier-naming)
    class UnionOperator : public core::physical::operators::pipe::TypedPipeOperator {
    public:
        UnionOperator(core::physical::port::Port inputPort, core::physical::port::Port outputPort);

        void consume(core::physical::segment::SegmentContext& segmentCtx,
                     core::shared::identifiers::PortId /*inputPort*/,
                     const core::physical::data::DataPackage& tuple,
                     const core::physical::segment::SegmentDataConsumers& consumers) override;

        void declareState(core::physical::state::StateRegistry &stateRegistry, llvm::Module &module,
                          llvm::LLVMContext &context) override;

        void consumePunctuation(core::physical::segment::SegmentContext &segmentCtx,
                                core::shared::identifiers::PortId inputPort,
                                const core::physical::data::PunctuationPackage &punctuation,
                                const core::physical::segment::SegmentDataConsumers &consumers) override;

    private:
        core::physical::state::StateHandle mWatermarkState;
    };
}

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_PHYSICAL_UNION_OPERATOR_HPP