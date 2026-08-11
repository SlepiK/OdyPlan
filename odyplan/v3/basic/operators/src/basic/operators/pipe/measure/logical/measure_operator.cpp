#include <basic/operators/pipe/measure/logical/measure_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::measure::logical {
    MeasureOperator::MeasureOperator(std::string label) : mLabel(std::move(label)) {}

    core::shared::types::OperatorTypeId MeasureOperator::getOperatorTypeId() const {
        return TYPE_ID;
    }

    const std::string& MeasureOperator::getLabel() const {
        return mLabel;
    }

    void MeasureOperator::validateInput() {}
}  // namespace sierra::odyplan::v3::basic::operators::pipe::measure::logical