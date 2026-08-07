#include <basic/operators/pipe/window/logical/count_window_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::logical {
    CountWindowOperator::CountWindowOperator(const basic::window::count::CountWindowConfig &windowConfig)
            : mWindowConfig(windowConfig) {
        checkBaseConfig(mWindowConfig.type, mWindowConfig.size, mWindowConfig.slide);
    }

    core::shared::types::OperatorTypeId CountWindowOperator::getOperatorTypeId() const {
        return TYPE_ID;
    }

    const basic::window::count::CountWindowConfig &CountWindowOperator::getWindowConfig() const {
        return mWindowConfig;
    }

    void CountWindowOperator::validateInput() {
    }
}