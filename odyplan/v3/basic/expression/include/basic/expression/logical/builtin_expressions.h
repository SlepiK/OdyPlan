#ifndef SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_LOGICAL_BUILTIN_EXPRESSION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_LOGICAL_BUILTIN_EXPRESSION_HPP

#include <basic/expression/logical/compare_expression.h>
#include <basic/expression/logical/constant_expression.h>
#include <basic/expression/logical/expression.h>
#include <basic/expression/logical/field_reference_expression.h>
#include <basic/expression/logical/logical_expression.h>

namespace sierra::odyplan::v3::basic::expression::logical {
    // Helpers
    inline std::unique_ptr<IExpression> field(const std::string& fieldName) {
        return std::make_unique<FieldReferenceExpression>(fieldName);
    }

    inline std::unique_ptr<IExpression> field(std::size_t portIndex, const std::string& fieldName) {
        return std::make_unique<FieldReferenceExpression>(fieldName, portIndex);
    }

    inline std::unique_ptr<IExpression> constant(core::shared::types::ValueTypeId typeId,
                                                 const std::string& value) {
        return std::make_unique<ConstantExpression>(typeId, value);
    }

    inline std::unique_ptr<IExpression> cInt(const int value) {
        return constant(static_cast<core::shared::types::ValueTypeId>(
                            core::shared::types::BuiltinValueType::INT32),
                        std::to_string(value));
    }

    inline std::unique_ptr<IExpression> cFloat(const float value) {
        return constant(static_cast<core::shared::types::ValueTypeId>(
                            core::shared::types::BuiltinValueType::FLOAT),
                        std::to_string(value));
    }

    inline std::unique_ptr<IExpression> cDouble(const double value) {
        return constant(static_cast<core::shared::types::ValueTypeId>(
                            core::shared::types::BuiltinValueType::DOUBLE),
                        std::to_string(value));
    }

    inline std::unique_ptr<IExpression> cBool(const bool value) {
        return constant(static_cast<core::shared::types::ValueTypeId>(
                            core::shared::types::BuiltinValueType::BOOL),
                        std::to_string(static_cast<int>(value)));
    }

    inline std::unique_ptr<IExpression> cString(const std::string& value) {
        return constant(static_cast<core::shared::types::ValueTypeId>(
                            core::shared::types::BuiltinValueType::STRING),
                        value);
    }

    //

    // NOLINTBEGIN
    inline std::unique_ptr<IExpression> EQ(std::unique_ptr<IExpression> leftPart,
                                           std::unique_ptr<IExpression> rightPart) {
        return std::make_unique<CompareExpression>(
            std::move(leftPart), CompareOp::EQ, std::move(rightPart));
    }

    inline std::unique_ptr<IExpression> NE(std::unique_ptr<IExpression> leftPart,
                                           std::unique_ptr<IExpression> rightPart) {
        return std::make_unique<CompareExpression>(
            std::move(leftPart), CompareOp::NE, std::move(rightPart));
    }

    inline std::unique_ptr<IExpression> LT(std::unique_ptr<IExpression> leftPart,
                                           std::unique_ptr<IExpression> rightPart) {
        return std::make_unique<CompareExpression>(
            std::move(leftPart), CompareOp::LT, std::move(rightPart));
    }

    inline std::unique_ptr<IExpression> LE(std::unique_ptr<IExpression> leftPart,
                                           std::unique_ptr<IExpression> rightPart) {
        return std::make_unique<CompareExpression>(
            std::move(leftPart), CompareOp::LE, std::move(rightPart));
    }

    inline std::unique_ptr<IExpression> GT(std::unique_ptr<IExpression> leftPart,
                                           std::unique_ptr<IExpression> rightPart) {
        return std::make_unique<CompareExpression>(
            std::move(leftPart), CompareOp::GT, std::move(rightPart));
    }

    inline std::unique_ptr<IExpression> GE(std::unique_ptr<IExpression> leftPart,
                                           std::unique_ptr<IExpression> rightPart) {
        return std::make_unique<CompareExpression>(
            std::move(leftPart), CompareOp::GE, std::move(rightPart));
    }

    inline std::unique_ptr<IExpression> AND(std::unique_ptr<IExpression> leftPart,
                                            std::unique_ptr<IExpression> rightPart) {
        return std::make_unique<LogicalExpression>(
            std::move(leftPart), LogicalOp::AND, std::move(rightPart));
    }

    inline std::unique_ptr<IExpression> OR(std::unique_ptr<IExpression> leftPart,
                                           std::unique_ptr<IExpression> rightPart) {
        return std::make_unique<LogicalExpression>(
            std::move(leftPart), LogicalOp::OR, std::move(rightPart));
    }

    inline std::unique_ptr<IExpression> NOT(std::unique_ptr<IExpression> oprAffected) {
        return std::make_unique<LogicalExpression>(LogicalOp::NOT,
                                                   std::move(oprAffected));
    }

    // NOLINTEND
}  // namespace sierra::odyplan::v3::basic::expression

#endif  // SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_LOGICAL_BUILTIN_EXPRESSION_HPP