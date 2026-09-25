#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_PHYSICAL_UNION_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_PHYSICAL_UNION_OPERATOR_HPP

#include <core/logical/port/port.h>
#include <core/physical/operators/pipe/typed_pipe_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::union_::physical {// NOLINT(readability-identifier-naming)
    class UnionOperator : public core::physical::operators::pipe::TypedPipeOperator {
    public:
        UnionOperator(core::physical::port::Port inputPort, core::physical::port::Port outputPort, core::logical::port::Port schema, bool outOfOrder);

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
        bool mOutOfOrder;
        core::logical::port::Port mSchema;
        core::physical::state::StateHandle mWatermarkState;
        core::physical::state::StateHandle mUnionBufferState;

        void flushBuffer(core::physical::segment::SegmentContext& segmentCtx, const core::physical::segment::SegmentDataConsumers& consumers);

        void emitBufferedEntry(core::physical::segment::SegmentContext& segmentCtx, const core::physical::segment::SegmentDataConsumers& consumers, llvm::Value* index);


        static llvm::GlobalVariable* buildTypeIds(llvm::Module& module,
                                                  llvm::LLVMContext& ctx,
                                                  const core::logical::port::Port& schema,
                                                  const std::string& name);
    };
}

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_PHYSICAL_UNION_OPERATOR_HPP