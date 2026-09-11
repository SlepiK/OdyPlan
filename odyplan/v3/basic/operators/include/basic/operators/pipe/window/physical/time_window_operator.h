#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_PHYSICAL_TIME_WINDOW_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_PHYSICAL_TIME_WINDOW_OPERATOR_HPP

#include <core/physical/operators/pipe/typed_pipe_operator.h>
#include <basic/window/time/time_window_config.h>
#include <core/logical/port/port.h>
#include <basic/window/time/time_window_type.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::physical {
    class TimeWindowOperator final : public core::physical::operators::pipe::TypedPipeOperator {
    public:
        TimeWindowOperator(core::physical::port::Port inputPort,
                           core::physical::port::Port outputPort,
                           const basic::window::time::TimeWindowConfig &config,
                           core::logical::port::Port schema);

        void consume(core::physical::segment::SegmentContext &segmentCtx,
                     core::shared::identifiers::PortId inputPort,
                     const core::physical::data::DataPackage &tuple,
                     const core::physical::segment::SegmentDataConsumers &consumers) override;

        void consumePunctuation(core::physical::segment::SegmentContext &segmentCtx,
                                core::shared::identifiers::PortId inputPort,
                                const core::physical::data::PunctuationPackage &punctuation,
                                const core::physical::segment::SegmentDataConsumers &consumers) override;

    private:
        basic::window::time::TimeWindowConfig mConfig;
        core::logical::port::Port mDataSchema;
        core::shared::identifiers::OperatorId mLogicalOperatorId = 0;
        core::physical::state::StateHandle mState;
        llvm::Function *mDrainFn = nullptr;
        basic::window::TimeWindowType mWindowType;

        llvm::Value* computeWindowEnd(core::physical::segment::SegmentContext& segmentCtx, llvm::Value* eventTs) const;
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::physical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_PHYSICAL_TIME_WINDOW_OPERATOR_HPP
