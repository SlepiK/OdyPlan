#ifndef SIERRA_ODYPLAN_V2_CORE_COMMON_EXPRESSION_EXPRESSION_H
#define SIERRA_ODYPLAN_V2_CORE_COMMON_EXPRESSION_EXPRESSION_H

#include <cstddef>
#include <memory>

#include "core/common/type/type.h"

namespace sierra::odyplan::v2::core::common::expression {
    enum class ExpressionType {
        FIELDREFERENCE,
        CONSTANT,
        COMPARE,
        LOGICAL
    };

    class IExpression {
    public:
        virtual ~IExpression() = default;
        [[nodiscard]] virtual ExpressionType getExpressionType() const = 0;
    };

    class FieldReferenceExpression : public IExpression {
    public:
        explicit FieldReferenceExpression(std::size_t fieldIndex)
            : mFieldIndex(fieldIndex) {
        }

        [[nodiscard]] ExpressionType getExpressionType() const override {
            return ExpressionType::FIELDREFERENCE;
        }

        [[nodiscard]] std::size_t getFieldIndex() const {
            return mFieldIndex;
        }

    private:
        std::size_t mFieldIndex;
    };

    class ConstantExpression : public IExpression {
    public:
        explicit ConstantExpression(type::TypeId typeId, std::string serializedValue)
            : mTypeId(typeId),
                mSerializedValue(std::move(serializedValue)) {
        }

        [[nodiscard]] ExpressionType getExpressionType() const override {
            return ExpressionType::CONSTANT;
        }

        [[nodiscard]] type::TypeId getTypeId() const {
            return mTypeId;
        }

        [[nodiscard]] const std::string& getSerializedValue() const {
            return mSerializedValue;
        }

    private:
        type::TypeId mTypeId;
        std::string mSerializedValue;
    };

    class CompareExpression : public IExpression {
    public:
        enum class CompareOp {
            EQ, //EQUAL
            NE, //NOT_EQUAL
            LT, //LESS_THAN
            LE, //LESS_THAN_OR_EQUAL
            GT, //GREATER_THAN
            GE, //GREATER_THAN_OR_EQUAL
        };

        explicit CompareExpression(std::unique_ptr<IExpression> leftPart, CompareOp opr, std::unique_ptr<IExpression> rightPart)
            : mLeftPart(std::move(leftPart)),
                mRightPart(std::move(rightPart)),
                mOpr(opr) {
        }

        [[nodiscard]] ExpressionType getExpressionType() const override {
            return ExpressionType::COMPARE;
        }

        [[nodiscard]] const IExpression& getLeftPart() const {
            return *mLeftPart;
        }

        [[nodiscard]] const IExpression& getRightPart() const {
            return *mRightPart;
        }

        [[nodiscard]] CompareOp getOpr() const {
            return mOpr;
        }

    private:
        std::unique_ptr<IExpression> mLeftPart;
        std::unique_ptr<IExpression> mRightPart;
        CompareOp mOpr;
    };


}  // namespace sierra::odyplan::v2::core::common::expression

#endif  // SIERRA_ODYPLAN_V2_CORE_COMMON_EXPRESSION_EXPRESSION_H