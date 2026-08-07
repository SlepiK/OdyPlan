#ifndef SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_BUILTIN_EXPRESSION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_BUILTIN_EXPRESSION_HPP

#include <basic/expression/expression.h>
#include <basic/expression/compare_expression.h>
#include <basic/expression/constant_expression.h>
#include <basic/expression/field_reference_expression.h>
#include <basic/expression/logical_expression.h>

namespace sierra::odyplan::v3::basic::expression {
    // Helpers
    inline std::unique_ptr<IExpression> field(std::size_t fieldIndex) {
        return std::make_unique<FieldReferenceExpression>(fieldIndex);
    }

    inline std::unique_ptr<IExpression> constant(core::shared::types::ValueTypeId typeId, const std::string& value) {
        return std::make_unique<ConstantExpression>(typeId, value);
    }

    inline std::unique_ptr<IExpression> cInt(const int value) {
        return constant(static_cast<core::shared::types::ValueTypeId>(core::shared::types::BuiltinValueType::INT32), std::to_string(value));
    }

    inline std::unique_ptr<IExpression> cFloat(const float value) {
        return constant(static_cast<core::shared::types::ValueTypeId>(core::shared::types::BuiltinValueType::FLOAT), std::to_string(value));
    }

    inline std::unique_ptr<IExpression> cDouble(const double value) {
        return constant(static_cast<core::shared::types::ValueTypeId>(core::shared::types::BuiltinValueType::DOUBLE), std::to_string(value));
    }

    inline std::unique_ptr<IExpression> cBool(const bool value) {
        return constant(static_cast<core::shared::types::ValueTypeId>(core::shared::types::BuiltinValueType::BOOL), std::to_string(static_cast<int>(value)));
    }

    inline std::unique_ptr<IExpression> cString(const std::string& value) {
        return constant(static_cast<core::shared::types::ValueTypeId>(core::shared::types::BuiltinValueType::STRING), value);
    }

    //

    // NOLINTBEGIN
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
    // NOLINTEND
}

#endif //SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_BUILTIN_EXPRESSION_HPP