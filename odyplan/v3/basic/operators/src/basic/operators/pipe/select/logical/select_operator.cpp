#include <basic/operators/pipe/select/logical/select_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::select::logical {
    SelectOperator::SelectOperator(std::unique_ptr<basic::expression::logical::IExpression> predicate)
        : mLPredicate(std::move(predicate)) {}

    [[nodiscard]] core::shared::types::OperatorTypeId SelectOperator::getOperatorTypeId() const {
        return TYPE_ID;
    }

    [[nodiscard]] std::unique_ptr<basic::expression::physical::IExpression> SelectOperator::getPredicate() {
        if (mPPredicate == nullptr) {
            return nullptr;
        }

        return std::move(mPPredicate);
    }

    void SelectOperator::validateInput() {
        if (mLPredicate == nullptr) {
            throw std::invalid_argument("SelectOperator: predicate must be non-null");
        }

        if (mLPredicate->getTypeId() != std::to_underlying(core::shared::types::BuiltinValueType::BOOL)) {
            throw std::invalid_argument("SelectOperator: predicate must return bool");
        }

        const auto inputPort = getInputPorts().front();
        const core::logical::port::PortDescriptionResolver resolver(inputPort,{inputPort.size()});
        expression::translate::LogicalToPhysicalTranslator translator(resolver);
        mPPredicate = translator.translate(*mLPredicate);
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::select::logical