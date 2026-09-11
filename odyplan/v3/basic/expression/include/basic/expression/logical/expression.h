#ifndef SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_LOGICAL_EXPRESSION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_LOGICAL_EXPRESSION_HPP

#include <basic/expression/logical/expression_type.h>

namespace sierra::odyplan::v3::basic::expression::logical {
    class IExpression {
    public:
        IExpression() = default;

        virtual ~IExpression() = default;

        IExpression(IExpression const&) = delete;

        IExpression& operator=(IExpression const&) = delete;

        IExpression(IExpression&&) = delete;

        IExpression& operator=(IExpression&&) = delete;

        [[nodiscard]] virtual ExpressionType getExpressionType() const = 0;
    };
}  // namespace sierra::odyplan::v3::basic::expression

#endif  // SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_LOGICAL_EXPRESSION_HPP