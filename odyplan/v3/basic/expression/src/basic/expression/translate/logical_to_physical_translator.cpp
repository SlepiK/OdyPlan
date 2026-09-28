#include <basic/expression/translate/logical_to_physical_translator.h>

#include "basic/expression/logical/compare_expression.h"
#include "basic/expression/logical/constant_expression.h"
#include "basic/expression/logical/field_reference_expression.h"
#include "basic/expression/logical/logical_expression.h"
#include "basic/expression/physical/compare_expression.h"
#include "basic/expression/physical/constant_expression.h"
#include "basic/expression/physical/field_reference_expression.h"
#include "basic/expression/physical/logical_expression.h"

namespace sierra::odyplan::v3::basic::expression::translate {
    LogicalToPhysicalTranslator::LogicalToPhysicalTranslator(core::logical::port::PortDescriptionResolver resolver) : mResolver(std::move(resolver)) {
    }

    std::unique_ptr<physical::IExpression> LogicalToPhysicalTranslator::translate(logical::IExpression &expr) {
        switch (expr.getExpressionType()) {
            case logical::ExpressionType::FIELD_REFERENCE: {
                // NOLINTNEXTLINE
                auto& fieldRef = static_cast<logical::FieldReferenceExpression&>(expr);

                const std::size_t index = mResolver.resolve(fieldRef.getFieldName(), fieldRef.portIndex());
                const auto typeId = mResolver.getTypeId(index);
                fieldRef.mTypeId = typeId;

                return std::make_unique<physical::FieldReferenceExpression>(typeId, index);
            }

            case logical::ExpressionType::CONSTANT: {
                // NOLINTNEXTLINE
                const auto& constant = static_cast<logical::ConstantExpression&>(expr);

                return std::make_unique<physical::ConstantExpression>(constant.getTypeId(), constant.getSerializedValue());
            }

            case logical::ExpressionType::COMPARE: {
                // NOLINTNEXTLINE
                const auto& compare = static_cast<logical::CompareExpression&>(expr);

                auto left = translate(compare.getLeftPart());
                auto right = translate(compare.getRightPart());

                if (compare.getLeftPart().getTypeId() != compare.getRightPart().getTypeId()) {
                    throw std::invalid_argument("LogicalToPhysicalTranslator: unsupported expression type");
                }

                if (!core::shared::types::isPrimitiv(compare.getLeftPart().getTypeId()) && compare.getLeftPart().getTypeId() != std::to_underlying(core::shared::types::BuiltinValueType::STRING)) {
                    throw std::invalid_argument("LogicalToPhysicalTranslator: unsupported expression type");
                }

                if (!core::shared::types::isPrimitiv(compare.getRightPart().getTypeId()) && compare.getRightPart().getTypeId() != std::to_underlying(core::shared::types::BuiltinValueType::STRING)) {
                    throw std::invalid_argument("LogicalToPhysicalTranslator: unsupported expression type");
                }

                return std::make_unique<physical::CompareExpression>(std::move(left), compare.getOpr(), std::move(right));
            }

            case logical::ExpressionType::LOGICAL: {
                // NOLINTNEXTLINE
                const auto& logical = static_cast<logical::LogicalExpression&>(expr);

                auto left = translate(logical.getLeftPart());

                if (logical.getLeftPart().getTypeId() != std::to_underlying(core::shared::types::BuiltinValueType::BOOL)) {
                    throw std::invalid_argument("LogicalToPhysicalTranslator: unsupported expression");
                }

                if (logical.isUnary()) {
                    return std::make_unique<physical::LogicalExpression>(logical.getOpr(), std::move(left));
                }

                auto right = translate(*logical.getRightPart());

                if (logical.getRightPart()->getTypeId() != std::to_underlying(core::shared::types::BuiltinValueType::BOOL)) {
                    throw std::invalid_argument("LogicalToPhysicalTranslator: unsupported expression");
                }

                return std::make_unique<physical::LogicalExpression>(std::move(left), logical.getOpr(), std::move(right));
            }

            default:
                throw std::invalid_argument("LogicalToPhysicalTranslator: unsupported expression type");
        }
    }
}
