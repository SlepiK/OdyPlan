#ifndef SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_PHYSICAL_FIELD_REFERENCE_EXPRESSION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_PHYSICAL_FIELD_REFERENCE_EXPRESSION_HPP

#include <basic/expression/physical/expression.h>
#include <cstddef>

#include "core/shared/types/value_type_id.h"

namespace sierra::odyplan::v3::basic::expression::physical {
    class FieldReferenceExpression : public IExpression {
    public:
        explicit FieldReferenceExpression(core::shared::types::ValueTypeId typeId, std::size_t fieldIndex);

        [[nodiscard]] ExpressionType getExpressionType() const override;

        [[nodiscard]] std::size_t getFieldIndex() const;

        [[nodiscard]] core::shared::types::ValueTypeId getTypeId() const;

    private:
        core::shared::types::ValueTypeId mTypeId;
        std::size_t mFieldIndex;
    };
}  // namespace sierra::odyplan::v3::basic::expression

#endif  // SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_PHYSICAL_FIELD_REFERENCE_EXPRESSION_HPP
