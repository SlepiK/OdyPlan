#include <basic/operators/pipe/window/logical/time_window_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::logical {
    TimeWindowOperator::TimeWindowOperator(
        const basic::window::time::TimeWindowConfig& windowConfig)
        : mWindowConfig(windowConfig) {
        checkBaseConfig(mWindowConfig.type, mWindowConfig.size, mWindowConfig.slide);
    }

    core::shared::types::OperatorTypeId TimeWindowOperator::getOperatorTypeId() const {
        return TYPE_ID;
    }

    const basic::window::time::TimeWindowConfig& TimeWindowOperator::getWindowConfig() const {
        return mWindowConfig;
    }

    void TimeWindowOperator::validateInput() {
        if (mWindowConfig.size.count() <= 0) {
            throw std::invalid_argument("TimeWindowOperator::validateInput: size must be positive");
        }
        if (mWindowConfig.slide.count() <= 0) {
            throw std::invalid_argument("TimeWindowOperator::validateInput: slide must be positive");
        }
        if (mWindowConfig.slide > mWindowConfig.size) {
            throw std::invalid_argument("TimeWindowOperator::validateInput: slide must not exceed size");
        }
        if (mWindowConfig.allowedLateness.count() < 0) {
            throw std::invalid_argument("TimeWindowOperator::validateInput: allowedLateness must be non-negative");
        }
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::logical