#ifndef SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_EXPRESSION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_EXPRESSION_HPP

#include <basic/expression/expression_type.h>

namespace sierra::odyplan::v3::basic::expression {
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

#endif  // SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_EXPRESSION_HPP
