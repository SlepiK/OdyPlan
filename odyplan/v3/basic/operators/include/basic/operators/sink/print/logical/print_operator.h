#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_LOGICAL_PRINT_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_LOGICAL_PRINT_OPERATOR_HPP

#include <core/logical/operators/sink/typed_sink_operator.h>
#include <utility>

namespace sierra::odyplan::v3::basic::operators::sink::print::logical {
    class PrintOperator : public core::logical::operators::sink::TypedSinkOperator {
    public:
        using PortFieldIndexV = std::vector<std::size_t>;
        using PortFieldNameV = std::vector<std::string>;

        static constexpr core::shared::types::OperatorTypeId TYPE_ID =
            std::to_underlying(core::shared::types::BuiltinOperatorType::PRINT);

        explicit PrintOperator(PortFieldIndexV indices);

        explicit PrintOperator(PortFieldNameV names);

        [[nodiscard]] core::shared::types::OperatorTypeId getOperatorTypeId() const override;

        [[nodiscard]] PortFieldIndexV getPortFieldIndices() const;

    protected:
        void validateInput() override;

    private:
        PortFieldIndexV mPortFieldIndices;
        PortFieldNameV mPortFieldNames;
    };
}  // namespace sierra::odyplan::v3::basic::operators::sink::print::logical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_LOGICAL_PRINT_OPERATOR_HPP
