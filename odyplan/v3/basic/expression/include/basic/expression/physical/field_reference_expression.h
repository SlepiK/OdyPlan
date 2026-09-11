#ifndef SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_PHYSICAL_FIELD_REFERENCE_EXPRESSION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_PHYSICAL_FIELD_REFERENCE_EXPRESSION_HPP

#include <basic/expression/physical/expression.h>
#include <cstddef>

namespace sierra::odyplan::v3::basic::expression::physical {
    class FieldReferenceExpression : public IExpression {
    public:
        explicit FieldReferenceExpression(std::size_t fieldIndex);

        [[nodiscard]] ExpressionType getExpressionType() const override;

        [[nodiscard]] std::size_t getFieldIndex() const;

    private:
        std::size_t mFieldIndex;
    };
}  // namespace sierra::odyplan::v3::basic::expression

#endif  // SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_PHYSICAL_FIELD_REFERENCE_EXPRESSION_HPP
