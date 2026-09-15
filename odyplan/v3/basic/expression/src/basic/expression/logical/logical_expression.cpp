#include <utility>
#include <basic/expression/logical/logical_expression.h>

namespace sierra::odyplan::v3::basic::expression::logical {
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

    IExpression& LogicalExpression::getLeftPart() const {
        return *mLeftPart;
    }

    IExpression* LogicalExpression::getRightPart() const {
        return mRightPart.get();
    }

    LogicalOp LogicalExpression::getOpr() const {
        return mOpr;
    }

    bool LogicalExpression::isUnary() const {
        return mRightPart == nullptr;
    }

    core::shared::types::ValueTypeId LogicalExpression::getTypeId() const {
        return std::to_underlying(core::shared::types::BuiltinValueType::BOOL);
    }
}  // namespace sierra::odyplan::v3::basic::expression
