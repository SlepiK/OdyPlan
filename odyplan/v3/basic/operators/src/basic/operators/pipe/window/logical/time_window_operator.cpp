#include <basic/operators/pipe/window/logical/time_window_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::logical {
    TimeWindowOperator::TimeWindowOperator(const basic::window::time::TimeWindowConfig &windowConfig)
            : mWindowConfig(windowConfig) {
        checkBaseConfig(mWindowConfig.type, mWindowConfig.size, mWindowConfig.slide);
    }

    core::shared::types::OperatorTypeId TimeWindowOperator::getOperatorTypeId() const {
        return TYPE_ID;
    }

    const basic::window::time::TimeWindowConfig &TimeWindowOperator::getWindowConfig() const {
        return mWindowConfig;
    }

    void TimeWindowOperator::validateInput() {
        if (mWindowConfig.allowedLateness.count() < 0) {
            throw std::invalid_argument("allowedLateness must be non-negative");
        }

        if (mWindowConfig.timestampIndex >= getInputPorts().at(0).size()) {
            throw std::invalid_argument("timestamp index out of range");
        }

        if (getInputPorts().at(0).at(mWindowConfig.timestampIndex).valueTypeId != std::to_underlying(
                core::shared::types::BuiltinValueType::INT64)) {
            throw std::invalid_argument("timestamp must be int64");
                }
    }
}