#ifndef SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_LOGICAL_EXPRESSION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_LOGICAL_EXPRESSION_HPP

#include <cstdint>
#include <memory>
#include <basic/expression/expression.h>

namespace sierra::odyplan::v3::basic::expression
{
    class LogicalExpression : public IExpression {
    public:
        enum class LogicalOp : std::uint8_t {
            AND,
            OR,
            NOT,
        };

        LogicalExpression(std::unique_ptr<IExpression> leftPart, LogicalOp opr, std::unique_ptr<IExpression> rightPart);

        LogicalExpression(LogicalOp opr, std::unique_ptr<IExpression> oprAffected);

        [[nodiscard]] ExpressionType getExpressionType() const override;

        [[nodiscard]] const IExpression& getLeftPart() const;

        [[nodiscard]] const IExpression* getRightPart() const;

        [[nodiscard]] LogicalOp getOpr() const;

        [[nodiscard]] bool isUnary() const;

    private:
        std::unique_ptr<IExpression> mLeftPart;
        std::unique_ptr<IExpression> mRightPart;
        LogicalOp mOpr;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_LOGICAL_EXPRESSION_HPP