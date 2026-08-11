#include <basic/operators/pipe/select/logical/select_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::select::logical {
    SelectOperator::SelectOperator(std::unique_ptr<basic::expression::IExpression> predicate)
        : mPredicate(std::move(predicate)) {}

    [[nodiscard]] core::shared::types::OperatorTypeId SelectOperator::getOperatorTypeId() const {
        return TYPE_ID;
    }

    [[nodiscard]] const basic::expression::IExpression& SelectOperator::getPredicate() const {
        return *mPredicate;
    }

    void SelectOperator::validateInput() {
        if (mPredicate == nullptr) {
            throw std::invalid_argument("SelectOperator: predicate must be non-null");
        }
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::select::logical