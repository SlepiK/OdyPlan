#ifndef SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_FIELD_REFERENCE_EXPRESSION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_FIELD_REFERENCE_EXPRESSION_HPP

#include <cstddef>
#include <basic/expression/expression.h>

namespace sierra::odyplan::v3::basic::expression {
    class FieldReferenceExpression : public IExpression {
    public:
        explicit FieldReferenceExpression(std::size_t fieldIndex);

        [[nodiscard]] ExpressionType getExpressionType() const override;

        [[nodiscard]] std::size_t getFieldIndex() const;

    private:
        std::size_t mFieldIndex;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_FIELD_REFERENCE_EXPRESSION_HPP