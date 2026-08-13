#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_PHYSICAL_TIME_WINDOW_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_PHYSICAL_TIME_WINDOW_OPERATOR_HPP

#include <core/physical/operators/pipe/typed_pipe_operator.h>
#include <basic/window/time/time_window_config.h>
#include <core/logical/port/port.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::physical {
    class TimeWindowOperator final : public core::physical::operators::pipe::TypedPipeOperator {
    public:
        TimeWindowOperator(core::physical::port::Port inputPort,
                           core::physical::port::Port outputPort,
                           const basic::window::time::TimeWindowConfig& config);

        void declareState(core::physical::state::StateRegistry& stateRegistry,
                          llvm::Module& module,
                          llvm::LLVMContext& context) override;

        void consume(core::physical::segment::SegmentContext& segmentCtx,
                     core::shared::identifiers::PortId inputPort,
                     const core::physical::data::DataPackage& tuple,
                     const core::physical::segment::SegmentDataConsumers& consumers) override;

        void end(core::physical::segment::SegmentContext& segmentCtx,
                 core::shared::identifiers::PortId inputPort,
                 const core::physical::segment::SegmentDataConsumers& consumers) override;

    private:
        basic::window::time::TimeWindowConfig mConfig;
        core::logical::port::Port mDataSchema;
        core::shared::identifiers::OperatorId mLogicalOperatorId = 0;

        void codegenTumbling(core::physical::segment::SegmentContext& segmentCtx,
            const core::physical::data::DataPackage& tuple,
            const core::physical::segment::SegmentDataConsumers& consumers) const;

        void codegenUnrolled(core::physical::segment::SegmentContext& segmentCtx,
            const core::physical::data::DataPackage& tuple,
            const core::physical::segment::SegmentDataConsumers& consumers,
            std::uint64_t windowsPerTuple) const;

        void codegenLoop(core::physical::segment::SegmentContext& segmentCtx,
            const core::physical::data::DataPackage& tuple,
            const core::physical::segment::SegmentDataConsumers& consumers,
            std::uint64_t windowsPerTuple) const;

        static void emitWindow(core::physical::segment::SegmentContext& segmentCtx,
            const core::physical::data::DataPackage& tuple,
            llvm::Value* windowStart,
            llvm::Value* windowEnd,
            const core::physical::segment::SegmentDataConsumers& consumers) ;
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::physical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_PHYSICAL_TIME_WINDOW_OPERATOR_HPP
