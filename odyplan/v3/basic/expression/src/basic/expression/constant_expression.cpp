#include <basic/expression/constant_expression.h>

namespace sierra::odyplan::v3::basic::expression {
    ConstantExpression::ConstantExpression(const core::shared::types::ValueTypeId typeId,
                                           std::string serializedValue)
        : mTypeId(typeId), mSerializedValue(std::move(serializedValue)) {}

    ExpressionType ConstantExpression::getExpressionType() const {
        return ExpressionType::CONSTANT;
    }

    core::shared::types::ValueTypeId ConstantExpression::getTypeId() const {
        return mTypeId;
    }

    const std::string& ConstantExpression::getSerializedValue() const {
        return mSerializedValue;
    };
}  // namespace sierra::odyplan::v3::basic::expression
