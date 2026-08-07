#include <basic/expression/field_reference_expression.h>

namespace sierra::odyplan::v3::basic::expression
{
    FieldReferenceExpression::FieldReferenceExpression(const std::size_t fieldIndex)
        : mFieldIndex(fieldIndex)
    {
    }

    ExpressionType FieldReferenceExpression::getExpressionType() const
    {
        return ExpressionType::FIELD_REFERENCE;
    }

    std::size_t FieldReferenceExpression::getFieldIndex() const
    {
        return mFieldIndex;
    };
}
