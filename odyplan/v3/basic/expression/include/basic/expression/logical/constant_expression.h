#ifndef SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_LOGICAL_CONSTANT_EXPRESSION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_LOGICAL_CONSTANT_EXPRESSION_HPP

#include <basic/expression/logical/expression.h>
#include <core/shared/types/value_type_id.h>
#include <string>

namespace sierra::odyplan::v3::basic::expression::logical {
    class ConstantExpression : public IExpression {
    public:
        explicit ConstantExpression(core::shared::types::ValueTypeId typeId,
                                    std::string serializedValue);

        [[nodiscard]] ExpressionType getExpressionType() const override;

        [[nodiscard]] core::shared::types::ValueTypeId getTypeId() const override;

        [[nodiscard]] const std::string& getSerializedValue() const;

    private:
        core::shared::types::ValueTypeId mTypeId;
        std::string mSerializedValue;
    };
}  // namespace sierra::odyplan::v3::basic::expression

#endif  // SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_LOGICAL_CONSTANT_EXPRESSION_HPP
