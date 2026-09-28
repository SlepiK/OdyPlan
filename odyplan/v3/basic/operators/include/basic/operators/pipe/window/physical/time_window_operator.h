#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_PHYSICAL_TIME_WINDOW_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_PHYSICAL_TIME_WINDOW_OPERATOR_HPP

#include <core/physical/operators/pipe/typed_pipe_operator.h>
#include <basic/operators/pipe/window/details/time_window_options.h>
#include <core/logical/port/port.h>
#include <basic/operators/pipe/window/details/time_window_type.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::physical {
    class TimeWindowOperator final : public core::physical::operators::pipe::TypedPipeOperator {
    public:
        TimeWindowOperator(core::physical::port::Port inputPort,
                           core::physical::port::Port outputPort,
                           const details::TimeWindowOptions &config);

        void consume(core::physical::segment::SegmentContext &segmentCtx,
                     core::shared::identifiers::PortId inputPort,
                     const core::physical::data::DataPackage &tuple,
                     const core::physical::segment::SegmentDataConsumers &consumers) override;

    private:
        details::TimeWindowOptions mConfig;
        details::TimeWindowType mWindowType;

        llvm::Value* computeWindowEnd(core::physical::segment::SegmentContext& segmentCtx, llvm::Value* eventTs) const;
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::physical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_PHYSICAL_TIME_WINDOW_OPERATOR_HPP
