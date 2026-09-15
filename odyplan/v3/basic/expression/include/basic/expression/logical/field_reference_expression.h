#ifndef SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_LOGICAL_FIELD_REFERENCE_EXPRESSION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_LOGICAL_FIELD_REFERENCE_EXPRESSION_HPP

#include <basic/expression/logical/expression.h>
#include <cstddef>
#include <optional>
#include <string>

namespace sierra::odyplan::v3::basic::expression::translate {
    class LogicalToPhysicalTranslator;
}

namespace sierra::odyplan::v3::basic::expression::logical {

    class FieldReferenceExpression : public IExpression {
        friend class translate::LogicalToPhysicalTranslator;
    public:
        explicit FieldReferenceExpression(std::string fieldName);

        explicit FieldReferenceExpression(std::string fieldName, std::size_t portIndex);

        [[nodiscard]] ExpressionType getExpressionType() const override;

        [[nodiscard]] const std::string& getFieldName() const;

        [[nodiscard]] bool isPortQualified() const;

        [[nodiscard]] bool isQualified() const;

        [[nodiscard]] const std::optional<std::size_t>& portIndex() const;

        [[nodiscard]] core::shared::types::ValueTypeId getTypeId() const override;

    private:
        core::shared::types::ValueTypeId mTypeId;
        std::string mFieldName;
        std::optional<std::size_t> mPortIndex;
    };
}  // namespace sierra::odyplan::v3::basic::expression

#endif  // SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_LOGICAL_FIELD_REFERENCE_EXPRESSION_HPP
