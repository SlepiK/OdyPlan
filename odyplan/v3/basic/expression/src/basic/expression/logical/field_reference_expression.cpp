#include <basic/expression/logical/field_reference_expression.h>

namespace sierra::odyplan::v3::basic::expression::logical {
    FieldReferenceExpression::FieldReferenceExpression(std::string fieldName) : mFieldName(std::move(fieldName)) {}

    FieldReferenceExpression::FieldReferenceExpression(std::string fieldName, std::size_t portIndex) : mFieldName(std::move(fieldName)), mPortIndex(portIndex) {}

    ExpressionType FieldReferenceExpression::getExpressionType() const {
        return ExpressionType::FIELD_REFERENCE;
    }

    const std::string &FieldReferenceExpression::getFieldName() const {
        return mFieldName;
    }

    bool FieldReferenceExpression::isPortQualified() const {
        return mPortIndex.has_value();
    }

    bool FieldReferenceExpression::isQualified() const {
        return isPortQualified();
    }

    const std::optional<std::size_t>& FieldReferenceExpression::portIndex() const {
        return mPortIndex;
    }
}  // namespace sierra::odyplan::v3::basic::expression