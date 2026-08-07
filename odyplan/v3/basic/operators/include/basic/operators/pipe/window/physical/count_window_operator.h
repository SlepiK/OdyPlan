#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_PHYSICAL_COUNT_WINDOW_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_PHYSICAL_COUNT_WINDOW_OPERATOR_HPP

#include <core/logical/port/port.h>
#include <basic/window/count/count_window_config.h>
#include <basic/operators/pipe/window/physical/base_window_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::physical {
    class CountWindowOperator final : public WindowOperatorBase {
    public:
        CountWindowOperator(core::physical::port::Port inputPort, core::physical::port::Port outputPort,
                            const basic::window::count::CountWindowConfig& config, core::logical::port::Port schema);

        void declareState(core::physical::state::StateRegistry& stateRegistry, llvm::Module& module, llvm::LLVMContext& context) override;

    private:
        basic::window::count::CountWindowConfig mConfig;
        core::logical::port::Port mDataSchema;
        core::physical::state::StateHandle mState;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_PHYSICAL_COUNT_WINDOW_OPERATOR_HPP