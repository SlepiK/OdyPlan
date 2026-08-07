#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MEASURE_LOGICAL_MEASURE_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MEASURE_LOGICAL_MEASURE_OPERATOR_HPP

#include <utility>
#include <core/logical/operators/pipe/typed_pipe_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::measure::logical {
    class MeasureOperator : public core::logical::operators::pipe::TypedPipeOperator {
    public:
        static constexpr core::shared::types::OperatorTypeId TYPE_ID = std::to_underlying(
            core::shared::types::BuiltinOperatorType::MEASURE);

        explicit MeasureOperator(std::string label);

        [[nodiscard]] core::shared::types::OperatorTypeId getOperatorTypeId() const override;

        [[nodiscard]] const std::string &getLabel() const;

    protected:
        void validateInput() override;

    private:
        std::string mLabel;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MEASURE_LOGICAL_MEASURE_OPERATOR_HPP