#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_PHYSICAL_PRINT_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_PHYSICAL_PRINT_OPERATOR_HPP

#include <llvm/IR/Module.h>

#include <core/physical/operators/sink/typed_sink_operator.h>
#include <core/logical/port/port.h>
#include <basic/operators/sink/print/details/print_operator_options.h>

namespace sierra::odyplan::v3::basic::operators::sink::print::physical {
    class PrintOperator : public core::physical::operators::sink::TypedSinkOperator {
    public:
        using PortFieldIndexV = std::vector<std::size_t>;

        explicit PrintOperator(core::physical::port::Port inputPort, PortFieldIndexV indices, core::logical::port::Port lInputPort, details::PrintOperatorOptions options);

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
        core::logical::port::Port mSchema;
        details::PrintOperatorOptions mOptions;
    };
}  // namespace sierra::odyplan::v3::basic::operators::sink::print::physical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_PHYSICAL_PRINT_OPERATOR_HPP
