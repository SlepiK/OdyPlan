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
    LogicalToPhysicalTranslator::LogicalToPhysicalTranslator(const core::logical::port::PortDescriptionResolver &resolver) : mResolver(resolver) {
    }

    std::unique_ptr<physical::IExpression> LogicalToPhysicalTranslator::translate(const logical::IExpression &expr) {
        switch (expr.getExpressionType()) {
            case logical::ExpressionType::FIELD_REFERENCE: {
                const auto& fieldRef = static_cast<const logical::FieldReferenceExpression&>(expr);

                const std::size_t index = mResolver.resolve(fieldRef.getFieldName(), fieldRef.portIndex());

                return std::make_unique<physical::FieldReferenceExpression>(mResolver.getTypeId(index), index);
            }

            case logical::ExpressionType::CONSTANT: {
                const auto& constant = static_cast<const logical::ConstantExpression&>(expr);

                return std::make_unique<physical::ConstantExpression>(constant.getTypeId(), constant.getSerializedValue());
            }

            case logical::ExpressionType::COMPARE: {
                const auto& compare = static_cast<const logical::CompareExpression&>(expr);

                auto left = translate(compare.getLeftPart());
                auto right = translate(compare.getRightPart());

                return std::make_unique<physical::CompareExpression>(std::move(left), compare.getOpr(), std::move(right));
            }

            case logical::ExpressionType::LOGICAL: {
                const auto& logical = static_cast<const logical::LogicalExpression&>(expr);

                auto left = translate(logical.getLeftPart());

                if (logical.isUnary()) {
                    return std::make_unique<physical::LogicalExpression>(logical.getOpr(), std::move(left));
                }

                auto right = translate(*logical.getRightPart());

                return std::make_unique<physical::LogicalExpression>(std::move(left), logical.getOpr(), std::move(right));
            }

            default:
                throw std::invalid_argument("LogicalToPhysicalTranslator: unsupported expression type");
        }
    }
}
