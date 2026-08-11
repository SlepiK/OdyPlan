#include <basic/expression/logical_expression.h>

namespace sierra::odyplan::v3::basic::expression {
    LogicalExpression::LogicalExpression(std::unique_ptr<IExpression> leftPart,
                                         const LogicalOp opr,
                                         std::unique_ptr<IExpression> rightPart)
        : mLeftPart(std::move(leftPart)), mRightPart(std::move(rightPart)), mOpr(opr) {}

    LogicalExpression::LogicalExpression(const LogicalOp opr,
                                         std::unique_ptr<IExpression> oprAffected)
        : mLeftPart(std::move(oprAffected)), mRightPart(nullptr), mOpr(opr) {}

    ExpressionType LogicalExpression::getExpressionType() const {
        return ExpressionType::LOGICAL;
    }

    const IExpression& LogicalExpression::getLeftPart() const {
        return *mLeftPart;
    }

    const IExpression* LogicalExpression::getRightPart() const {
        return mRightPart.get();
    }

    LogicalExpression::LogicalOp LogicalExpression::getOpr() const {
        return mOpr;
    }

    bool LogicalExpression::isUnary() const {
        return mRightPart == nullptr;
    }
}  // namespace sierra::odyplan::v3::basic::expression
