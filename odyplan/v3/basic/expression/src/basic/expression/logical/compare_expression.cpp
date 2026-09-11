#include <basic/expression/logical/compare_expression.h>

namespace sierra::odyplan::v3::basic::expression::logical {
    CompareExpression::CompareExpression(std::unique_ptr<IExpression> leftPart,
                                         const CompareOp opr,
                                         std::unique_ptr<IExpression> rightPart)
        : mLeftPart(std::move(leftPart)), mRightPart(std::move(rightPart)), mOpr(opr) {}

    ExpressionType CompareExpression::getExpressionType() const {
        return ExpressionType::COMPARE;
    }

    const IExpression& CompareExpression::getLeftPart() const {
        return *mLeftPart;
    }

    const IExpression& CompareExpression::getRightPart() const {
        return *mRightPart;
    }

    CompareOp CompareExpression::getOpr() const {
        return mOpr;
    }
}  // namespace sierra::odyplan::v3::basic::expression
