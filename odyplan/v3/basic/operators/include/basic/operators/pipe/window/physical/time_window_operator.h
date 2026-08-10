#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_PHYSICAL_TIME_WINDOW_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_PHYSICAL_TIME_WINDOW_OPERATOR_HPP

#include <basic/operators/pipe/window/physical/base_window_operator.h>
#include <basic/window/time/time_window_config.h>
#include <core/logical/port/port.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::physical {
    class TimeWindowOperator final : public WindowOperatorBase {
    public:
        TimeWindowOperator(core::physical::port::Port inputPort,
                           core::physical::port::Port outputPort,
                           const basic::window::time::TimeWindowConfig& config,
                           core::logical::port::Port dataSchema);

        void declareState(core::physical::state::StateRegistry& stateRegistry,
                          llvm::Module& module,
                          llvm::LLVMContext& context) override;

    private:
        basic::window::time::TimeWindowConfig mConfig;
        core::logical::port::Port mDataSchema;
        core::shared::identifiers::OperatorId mLogicalOperatorId = 0;
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::physical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_PHYSICAL_TIME_WINDOW_OPERATOR_HPP
