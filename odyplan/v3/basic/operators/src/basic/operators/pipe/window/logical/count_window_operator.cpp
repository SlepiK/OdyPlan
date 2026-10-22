#include <basic/operators/pipe/window/logical/count_window_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::logical {
    CountWindowOperator::CountWindowOperator(
        const details::CountWindowOptions& windowConfig, const bool assureOrder = true)
        : mWindowConfig(windowConfig), mAssureOrder(assureOrder) {
    }

    core::shared::types::OperatorTypeId CountWindowOperator::getOperatorTypeId() const {
        return TYPE_ID;
    }

    const details::CountWindowOptions& CountWindowOperator::getWindowConfig() const {
        return mWindowConfig;
    }

    bool CountWindowOperator::isOutOfOrder() const {
        return !mAssureOrder;
    }

    void CountWindowOperator::validateInput() {}
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::logical