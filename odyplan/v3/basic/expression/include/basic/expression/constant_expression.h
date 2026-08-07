#ifndef SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_CONSTANT_EXPRESSION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_CONSTANT_EXPRESSION_HPP

#include <core/shared/types/value_type_id.h>
#include <string>
#include <basic/expression/expression.h>

namespace sierra::odyplan::v3::basic::expression
{
    class ConstantExpression : public IExpression {
    public:
        explicit ConstantExpression(core::shared::types::ValueTypeId typeId, std::string serializedValue);

        [[nodiscard]] ExpressionType getExpressionType() const override;

        [[nodiscard]] core::shared::types::ValueTypeId getTypeId() const;

        [[nodiscard]] const std::string& getSerializedValue() const;

    private:
        core::shared::types::ValueTypeId mTypeId;
        std::string mSerializedValue;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_CONSTANT_EXPRESSION_HPP