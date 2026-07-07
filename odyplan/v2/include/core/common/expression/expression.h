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

        explicit CompareExpression(std::unique_ptr<IExpression> leftPart, const CompareOp opr, std::unique_ptr<IExpression> rightPart)
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

    class LogicalExpression : public IExpression {
    public:
        enum class LogicalOp {
            AND,
            OR,
            NOT,
        };

        LogicalExpression(std::unique_ptr<IExpression> leftPart, const LogicalOp opr, std::unique_ptr<IExpression> rightPart)
            : mLeftPart(std::move(leftPart)),
                mRightPart(std::move(rightPart)),
                mOpr(opr) {
        }

        LogicalExpression(const LogicalOp opr, std::unique_ptr<IExpression> oprAffected)
            : mLeftPart(std::move(oprAffected)),
                mRightPart(nullptr),
                mOpr(opr) {
        }

        [[nodiscard]] ExpressionType getExpressionType() const override {
            return ExpressionType::LOGICAL;
        }

        [[nodiscard]] const IExpression& getLeftPart() const {
            return *mLeftPart;
        }

        [[nodiscard]] const IExpression* getRightPart() const {
            return mRightPart.get();
        }

        [[nodiscard]] LogicalOp getOpr() const {
            return mOpr;
        }

        [[nodiscard]] bool isUnary() const {
            return mRightPart == nullptr;
        }

    private:
        std::unique_ptr<IExpression> mLeftPart;
        std::unique_ptr<IExpression> mRightPart;
        LogicalOp mOpr;
    };

    // Helpers
    inline std::unique_ptr<IExpression> field(std::size_t fieldIndex) {
        return std::make_unique<FieldReferenceExpression>(fieldIndex);
    }

    inline std::unique_ptr<IExpression> constant(type::TypeId typeId, const std::string& value) {
        return std::make_unique<ConstantExpression>(typeId, value);
    }

    inline std::unique_ptr<IExpression> cInt(const int value) {
        return constant(static_cast<type::TypeId>(type::Type::INT), std::to_string(value));
    }

    inline std::unique_ptr<IExpression> cFloat(const float value) {
        return constant(static_cast<type::TypeId>(type::Type::FLOAT), std::to_string(value));
    }

    inline std::unique_ptr<IExpression> cDouble(const double value) {
        return constant(static_cast<type::TypeId>(type::Type::DOUBLE), std::to_string(value));
    }

    inline std::unique_ptr<IExpression> cBool(const bool value) {
        return constant(static_cast<type::TypeId>(type::Type::BOOL), std::to_string(value));
    }

    inline std::unique_ptr<IExpression> cString(const std::string& value) {
        return constant(static_cast<type::TypeId>(type::Type::STRING), value);
    }

    //

    inline std::unique_ptr<IExpression> EQ(std::unique_ptr<IExpression> leftPart, std::unique_ptr<IExpression> rightPart) {
        return std::make_unique<CompareExpression>(std::move(leftPart), CompareExpression::CompareOp::EQ, std::move(rightPart));
    }

    inline std::unique_ptr<IExpression> NE(std::unique_ptr<IExpression> leftPart, std::unique_ptr<IExpression> rightPart) {
        return std::make_unique<CompareExpression>(std::move(leftPart), CompareExpression::CompareOp::NE, std::move(rightPart));
    }

    inline std::unique_ptr<IExpression> LT(std::unique_ptr<IExpression> leftPart, std::unique_ptr<IExpression> rightPart) {
        return std::make_unique<CompareExpression>(std::move(leftPart), CompareExpression::CompareOp::LT, std::move(rightPart));
    }

    inline std::unique_ptr<IExpression> LE(std::unique_ptr<IExpression> leftPart, std::unique_ptr<IExpression> rightPart) {
        return std::make_unique<CompareExpression>(std::move(leftPart), CompareExpression::CompareOp::LE, std::move(rightPart));
    }

    inline std::unique_ptr<IExpression> GT(std::unique_ptr<IExpression> leftPart, std::unique_ptr<IExpression> rightPart) {
        return std::make_unique<CompareExpression>(std::move(leftPart), CompareExpression::CompareOp::GT, std::move(rightPart));
    }

    inline std::unique_ptr<IExpression> GE(std::unique_ptr<IExpression> leftPart, std::unique_ptr<IExpression> rightPart) {
        return std::make_unique<CompareExpression>(std::move(leftPart), CompareExpression::CompareOp::GE, std::move(rightPart));
    }

    inline std::unique_ptr<IExpression> AND(std::unique_ptr<IExpression> leftPart, std::unique_ptr<IExpression> rightPart) {
        return std::make_unique<LogicalExpression>(std::move(leftPart), LogicalExpression::LogicalOp::AND, std::move(rightPart));
    }

    inline std::unique_ptr<IExpression> OR(std::unique_ptr<IExpression> leftPart, std::unique_ptr<IExpression> rightPart) {
        return std::make_unique<LogicalExpression>(std::move(leftPart), LogicalExpression::LogicalOp::OR, std::move(rightPart));
    }

    inline std::unique_ptr<IExpression> NOT(std::unique_ptr<IExpression> oprAffected) {
        return std::make_unique<LogicalExpression>(LogicalExpression::LogicalOp::NOT, std::move(oprAffected));
    }

}  // namespace sierra::odyplan::v2::core::common::expression

#endif  // SIERRA_ODYPLAN_V2_CORE_COMMON_EXPRESSION_EXPRESSION_H