#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_LOGICAL_PRINT_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_LOGICAL_PRINT_OPERATOR_HPP

#include <core/logical/operators/sink/typed_sink_operator.h>
#include <utility>

#include <basic/operators/sink/print/details/print_operator_options.h>

namespace sierra::odyplan::v3::basic::operators::sink::print::logical {
    class PrintOperator : public core::logical::operators::sink::TypedSinkOperator {
    public:
        using PortFieldIndexV = std::vector<std::size_t>;
        using PortFieldNameV = std::vector<std::string>;

        static constexpr core::shared::types::OperatorTypeId TYPE_ID =
            std::to_underlying(core::shared::types::BuiltinOperatorType::PRINT);

        explicit PrintOperator(details::PrintOperatorOptions options);

        explicit PrintOperator(details::PrintOperatorOptions options, PortFieldIndexV indices);

        explicit PrintOperator(details::PrintOperatorOptions options, PortFieldNameV names);

        [[nodiscard]] core::shared::types::OperatorTypeId getOperatorTypeId() const override;

        [[nodiscard]] PortFieldIndexV getPortFieldIndices() const;

        [[nodiscard]] details::PrintOperatorOptions getOptions() const;

    protected:
        void validateInput() override;

    private:
        PortFieldIndexV mPortFieldIndices;
        PortFieldNameV mPortFieldNames;
        details::PrintOperatorOptions mOptions;
    };
}  // namespace sierra::odyplan::v3::basic::operators::sink::print::logical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_LOGICAL_PRINT_OPERATOR_HPP
