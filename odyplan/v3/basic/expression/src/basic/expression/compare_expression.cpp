#include <basic/expression/compare_expression.h>

namespace sierra::odyplan::v3::basic::expression
{
    CompareExpression::CompareExpression(std::unique_ptr<IExpression> leftPart, const CompareOp opr, std::unique_ptr<IExpression> rightPart)
            : mLeftPart(std::move(leftPart)),
                mRightPart(std::move(rightPart)),
                mOpr(opr) {
    }

    ExpressionType CompareExpression::getExpressionType() const override {
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
}