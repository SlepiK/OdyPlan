#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_PHYSICAL_PRINT_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_PHYSICAL_PRINT_OPERATOR_HPP

#include <llvm/IR/Module.h>

#include <core/physical/operators/sink/typed_sink_operator.h>

namespace sierra::odyplan::v3::basic::operators::sink::print::physical {
    class PrintOperator : public core::physical::operators::sink::TypedSinkOperator {
    public:
        using PortFieldIndexV = std::vector<std::size_t>;

        explicit PrintOperator(core::physical::port::Port inputPort, PortFieldIndexV indices);

        void consume(core::physical::segment::SegmentContext& segmentCtx,
                     core::shared::identifiers::PortId /*inputPort*/,
                     const core::physical::data::DataPackage& tuple,
                     const core::physical::segment::SegmentDataConsumers& /*consumers*/) override;

        void end(core::physical::segment::SegmentContext& segmentCtx,
                 core::shared::identifiers::PortId inputPort,
                 const core::physical::segment::SegmentDataConsumers& consumers) override;

        void consumePunctuation(core::physical::segment::SegmentContext &segmentCtx,
                                core::shared::identifiers::PortId inputPort,
                                const core::physical::data::PunctuationPackage &punctuation,
                                const core::physical::segment::SegmentDataConsumers &consumers) override;

    private:
        PortFieldIndexV mPortFieldIndices;

        static void
        codegenPrint(llvm::IRBuilder<>& builder, llvm::Module* module, const char* name);

        static void
        codegenPrintTimestamp(llvm::IRBuilder<>& builder, llvm::Module* module, llvm::Value* value);

        static void
        codegenPrintWindowMetadata(llvm::IRBuilder<>& builder, llvm::Module* module, llvm::Value* start, llvm::Value* end, llvm::Value* hasEnd, llvm::Value* isClosed);

        static void codegenPrintField(llvm::IRBuilder<>& builder,
                                      llvm::Module* module,
                                      core::shared::types::ValueTypeId typeId,
                                      llvm::Value* value);
    };
}  // namespace sierra::odyplan::v3::basic::operators::sink::print::physical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_PHYSICAL_PRINT_OPERATOR_HPP
