#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_DETAILS_JOIN_PREDMOD_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_DETAILS_JOIN_PREDMOD_HPP
#include <cstddef>
#include <optional>

#include <basic/expression/expression.h>
#include <basic/expression/compare_expression.h>
#include <basic/expression/field_reference_expression.h>
#include <core/shared/types/value_type_id.h>
#include <core/logical/port/port.h>

namespace sierra::odyplan::v3::basic::operators::pipe::join::physical::details {
    struct EquiKeyInfo {
        std::size_t leftFieldIndex;
        std::size_t rightFieldIndex;
        core::shared::types::ValueTypeId typeId;
    };

    inline std::optional<EquiKeyInfo> tryGetEquiKey(const basic::expression::IExpression* predicate,
        const core::logical::port::Port& leftSchema, const core::logical::port::Port& rightSchema) {
        if (predicate == nullptr) {
            return std::nullopt;
        }

        if (predicate->getExpressionType() != basic::expression::ExpressionType::COMPARE) {
            return std::nullopt;
        }

        //NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast,-warnings-as-errors)
        const auto& compare = static_cast<const basic::expression::CompareExpression&>(*predicate);

        if (compare.getOpr() != basic::expression::CompareExpression::CompareOp::EQ) {
            return std::nullopt;
        }

        const auto& left = compare.getLeftPart();

        if (left.getExpressionType() != basic::expression::ExpressionType::FIELD_REFERENCE) {
            return std::nullopt;
        }

        const auto& right = compare.getRightPart();

        if (right.getExpressionType() != basic::expression::ExpressionType::FIELD_REFERENCE) {
            return std::nullopt;
        }

        //NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast,-warnings-as-errors)
        const auto leftIndex = static_cast<const basic::expression::FieldReferenceExpression&>(left).getFieldIndex();
        //NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast,-warnings-as-errors)
        const auto rightIndex = static_cast<const basic::expression::FieldReferenceExpression&>(right).getFieldIndex();

        const auto leftSize = leftSchema.size();
        const auto rightSize = rightSchema.size();

        std::size_t leftFieldIndex = 0;
        std::size_t rightFieldIndex = 0;

        if (leftIndex < leftSize && rightIndex >= leftSize && rightIndex < leftSize + rightSize) {
            leftFieldIndex = leftIndex;
            rightFieldIndex = rightIndex - leftSize;
        } else if (rightIndex < leftSize && leftIndex >= leftSize && leftIndex < leftSize + rightSize) {
            leftFieldIndex = rightIndex;
            rightFieldIndex = leftIndex - leftSize;
        } else {
            return std::nullopt;
        }

        const auto leftTypeId = leftSchema.at(leftFieldIndex).valueTypeId;

        if (const auto rightTypeId = rightSchema.at(rightFieldIndex).valueTypeId; leftTypeId != rightTypeId) {
            return std::nullopt;
        }

        return EquiKeyInfo(leftFieldIndex, rightFieldIndex, leftTypeId);
    }
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_DETAILS_JOIN_PREDMOD_HPP