#ifndef SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_LOGICAL_LOGICAL_EXPRESSION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_LOGICAL_LOGICAL_EXPRESSION_HPP

#include <basic/expression/logical/expression.h>
#include <basic/expression/logical_operators.h>
#include <cstdint>
#include <memory>

namespace sierra::odyplan::v3::basic::expression::logical {
    class LogicalExpression : public IExpression {
    public:
        LogicalExpression(std::unique_ptr<IExpression> leftPart,
                          LogicalOp opr,
                          std::unique_ptr<IExpression> rightPart);

        LogicalExpression(LogicalOp opr, std::unique_ptr<IExpression> oprAffected);

        [[nodiscard]] ExpressionType getExpressionType() const override;

        [[nodiscard]] IExpression& getLeftPart() const;

        [[nodiscard]] IExpression* getRightPart() const;

        [[nodiscard]] LogicalOp getOpr() const;

        [[nodiscard]] bool isUnary() const;

        [[nodiscard]] core::shared::types::ValueTypeId getTypeId() const override;
    private:
        std::unique_ptr<IExpression> mLeftPart;
        std::unique_ptr<IExpression> mRightPart;
        LogicalOp mOpr;
    };
}  // namespace sierra::odyplan::v3::basic::expression

#endif  // SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_LOGICAL_LOGICAL_EXPRESSION_HPP