#ifndef SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_PHYSICAL_COMPARE_EXPRESSION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_PHYSICAL_COMPARE_EXPRESSION_HPP

#include <basic/expression/physical/expression.h>
#include <basic/expression/compare_operators.h>
#include <cstdint>
#include <memory>

namespace sierra::odyplan::v3::basic::expression::physical {
    class CompareExpression : public IExpression {
    public:
        explicit CompareExpression(std::unique_ptr<IExpression> leftPart,
                                   CompareOp opr,
                                   std::unique_ptr<IExpression> rightPart);

        [[nodiscard]] ExpressionType getExpressionType() const override;

        [[nodiscard]] const IExpression& getLeftPart() const;

        [[nodiscard]] const IExpression& getRightPart() const;

        [[nodiscard]] CompareOp getOpr() const;

    private:
        std::unique_ptr<IExpression> mLeftPart;
        std::unique_ptr<IExpression> mRightPart;
        CompareOp mOpr;
    };
}  // namespace sierra::odyplan::v3::basic::expression

#endif  // SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_PHYSICAL_COMPARE_EXPRESSION_HPP
