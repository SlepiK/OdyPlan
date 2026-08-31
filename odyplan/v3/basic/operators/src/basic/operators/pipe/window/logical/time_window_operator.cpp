#include <basic/operators/pipe/window/logical/time_window_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::logical {
    TimeWindowOperator::TimeWindowOperator(
        const basic::window::time::TimeWindowConfig& windowConfig)
        : mWindowConfig(windowConfig) {
        checkBaseConfig(mWindowConfig.type, mWindowConfig.size, mWindowConfig.slide, mWindowConfig.advance);
    }

    core::shared::types::OperatorTypeId TimeWindowOperator::getOperatorTypeId() const {
        return TYPE_ID;
    }

    const basic::window::time::TimeWindowConfig& TimeWindowOperator::getWindowConfig() const {
        return mWindowConfig;
    }

    void TimeWindowOperator::validateInput() {}
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::logical