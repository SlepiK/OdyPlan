#include <basic/operators/pipe/window/logical/time_window_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::logical {
    TimeWindowOperator::TimeWindowOperator(
        const details::TimeWindowOptions& windowConfig)
        : mWindowConfig(windowConfig) {
    }

    core::shared::types::OperatorTypeId TimeWindowOperator::getOperatorTypeId() const {
        return TYPE_ID;
    }

    const details::TimeWindowOptions& TimeWindowOperator::getWindowConfig() const {
        return mWindowConfig;
    }

    void TimeWindowOperator::validateInput() {}
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::logical