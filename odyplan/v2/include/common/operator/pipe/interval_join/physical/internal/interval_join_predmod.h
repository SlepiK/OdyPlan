#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_INTERVAL_JOIN_OPERATOR_PHYSICAL_INTERNAL_PREDMOD_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_INTERVAL_JOIN_OPERATOR_PHYSICAL_INTERNAL_PREDMOD_H
#include <cstddef>
#include <optional>

#include "core/common/expression/expression.h"
#include "core/common/identifier/type.h"
#include "core/logical/port/port.h"

namespace sierra::odyplan::v2::common::opr::pipe::physical::internal {
    struct EquiKeyInfo {
        std::size_t leftFieldIndex;
        std::size_t rightFieldIndex;
        core::common::identifier::TypeId typeId;
    };

    inline std::optional<EquiKeyInfo> tryGetEquiKey(const core::common::expression::IExpression* predicate,
        const core::logical::port::Port& leftSchema, const core::logical::port::Port& rightSchema) {
        if (predicate == nullptr) {
            return std::nullopt;
        }

        if (predicate->getExpressionType() != core::common::expression::ExpressionType::COMPARE) {
            return std::nullopt;
        }

        const auto& compare = static_cast<const core::common::expression::CompareExpression&>(*predicate);

        if (compare.getOpr() != core::common::expression::CompareExpression::CompareOp::EQ) {
            return std::nullopt;
        }

        const auto& left = compare.getLeftPart();

        if (left.getExpressionType() != core::common::expression::ExpressionType::FIELD_REFERENCE) {
            return std::nullopt;
        }

        const auto& right = compare.getRightPart();

        if (right.getExpressionType() != core::common::expression::ExpressionType::FIELD_REFERENCE) {
            return std::nullopt;
        }

        const auto leftIndex = static_cast<const core::common::expression::FieldReferenceExpression&>(left).getFieldIndex();
        const auto rightIndex = static_cast<const core::common::expression::FieldReferenceExpression&>(right).getFieldIndex();

        const auto leftSize = leftSchema.size();
        const auto rightSize = rightSchema.size();

        std::size_t leftFieldIndex, rightFieldIndex;

        if (leftIndex < leftSize && rightIndex >= leftSize && rightIndex < leftSize + rightSize) {
            leftFieldIndex = leftIndex;
            rightFieldIndex = rightIndex - leftSize;
        } else if (rightIndex < leftSize && leftIndex >= leftSize && leftIndex < leftSize + rightSize) {
            leftFieldIndex = rightIndex;
            rightFieldIndex = leftIndex - leftSize;
        } else {
            return std::nullopt;
        }

        const auto leftTypeId = leftSchema.at(leftFieldIndex).typeId;

        if (const auto rightTypeId = rightSchema.at(rightFieldIndex).typeId; leftTypeId != rightTypeId) {
            return std::nullopt;
        }

        return EquiKeyInfo(leftFieldIndex, rightFieldIndex, leftTypeId);
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_INTERVAL_JOIN_OPERATOR_PHYSICAL_INTERNAL_PREDMOD_H