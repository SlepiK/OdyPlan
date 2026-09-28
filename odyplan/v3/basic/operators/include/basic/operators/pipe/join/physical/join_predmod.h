#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_JOIN_PREDMOD_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_JOIN_PREDMOD_HPP
#include <basic/expression/physical/compare_expression.h>
#include <basic/expression/physical/expression.h>
#include <basic/expression/physical/field_reference_expression.h>
#include <core/logical/port/port.h>
#include <core/shared/types/value_type_id.h>
#include <cstddef>
#include <optional>
#include <basic/expression/physical/logical_expression.h>

namespace sierra::odyplan::v3::basic::operators::pipe::join::physical {
    struct EquiKeyInfo {
        std::size_t leftFieldIndex;
        std::size_t rightFieldIndex;
        core::shared::types::ValueTypeId typeId;
    };

    struct EquiKeyList {
        std::vector<EquiKeyInfo> keys;
    };

    inline bool pureAndEqui(const expression::physical::IExpression* expr, std::vector<const expression::physical::IExpression*>& pured) {
        if (expr == nullptr) {
            return false;
        }

        if (expr->getExpressionType() == expression::physical::ExpressionType::LOGICAL) {
            // NOLINTNEXTLINE
            const auto& logicalExp = static_cast<const expression::physical::LogicalExpression&>(*expr);
            if (logicalExp.isUnary() || logicalExp.getOpr() != expression::LogicalOp::AND) {
                return false;
            }

            const auto* rightExp = logicalExp.getRightPart();
            if (rightExp == nullptr) {
                return false;
            }

            return pureAndEqui(&logicalExp.getLeftPart(), pured) && pureAndEqui(rightExp, pured);
        }

        pured.push_back(expr);
        return true;
    }

    inline std::optional<EquiKeyInfo> tryGetEquiKey(const basic::expression::physical::IExpression* predicate,
                                                    const core::logical::port::Port& leftSchema,
                                                    const core::logical::port::Port& rightSchema) {
        if (predicate->getExpressionType() != basic::expression::physical::ExpressionType::COMPARE) {
            return std::nullopt;
        }

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast,-warnings-as-errors)
        const auto& compare = static_cast<const basic::expression::physical::CompareExpression&>(*predicate);

        if (compare.getOpr() != basic::expression::CompareOp::EQ) {
            return std::nullopt;
        }

        const auto& left = compare.getLeftPart();

        if (left.getExpressionType() != basic::expression::physical::ExpressionType::FIELD_REFERENCE) {
            return std::nullopt;
        }

        const auto& right = compare.getRightPart();

        if (right.getExpressionType() != basic::expression::physical::ExpressionType::FIELD_REFERENCE) {
            return std::nullopt;
        }

        // NOLINTBEGIN(cppcoreguidelines-pro-type-static-cast-downcast)
        const auto leftIndex =
            static_cast<const basic::expression::physical::FieldReferenceExpression&>(left).getFieldIndex();
        const auto rightIndex =
            static_cast<const basic::expression::physical::FieldReferenceExpression&>(right).getFieldIndex();
        // NOLINTEND(cppcoreguidelines-pro-type-static-cast-downcast)

        const auto leftSize = leftSchema.size();
        const auto rightSize = rightSchema.size();

        std::size_t leftFieldIndex = 0;
        std::size_t rightFieldIndex = 0;

        if (leftIndex < leftSize && rightIndex >= leftSize && rightIndex < leftSize + rightSize) {
            leftFieldIndex = leftIndex;
            rightFieldIndex = rightIndex - leftSize;
        } else if (rightIndex < leftSize && leftIndex >= leftSize &&
                   leftIndex < leftSize + rightSize) {
            leftFieldIndex = rightIndex;
            rightFieldIndex = leftIndex - leftSize;
        } else {
            return std::nullopt;
        }

        const auto leftTypeId = leftSchema.at(leftFieldIndex).valueTypeId;

        if (const auto rightTypeId = rightSchema.at(rightFieldIndex).valueTypeId;
            leftTypeId != rightTypeId) {
            return std::nullopt;
        }

        return EquiKeyInfo(leftFieldIndex, rightFieldIndex, leftTypeId);
    }

    inline std::optional<EquiKeyList> tryGetEquiKeyList(const basic::expression::physical::IExpression* predicate,
                                                    const core::logical::port::Port& leftSchema,
                                                    const core::logical::port::Port& rightSchema) {
        if (predicate == nullptr) {
            return std::nullopt;
        }

        std::vector<const expression::physical::IExpression*> parts;
        if (!pureAndEqui(predicate, parts)) {
            return std::nullopt;
        }

        EquiKeyList list;
        list.keys.reserve(parts.size());

        for (const auto* part: parts) {
            auto keyInfo = tryGetEquiKey(part, leftSchema, rightSchema);

            if (!keyInfo.has_value()) {
                return std::nullopt;
            }

            list.keys.push_back(*keyInfo);
        }

        return list;
    }
}

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_JOIN_PREDMOD_HPP