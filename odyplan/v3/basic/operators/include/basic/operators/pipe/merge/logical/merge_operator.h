#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MERGE_LOGICAL_MERGE_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MERGE_LOGICAL_MERGE_OPERATOR_HPP

#include <utility>
#include <vector>
#include <core/logical/operators/pipe/typed_pipe_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::merge::logical {
    class MergeOperator : public core::logical::operators::pipe::TypedPipeOperator {
    public:
        static constexpr core::shared::types::OperatorTypeId TYPE_ID = std::to_underlying(
            core::shared::types::BuiltinOperatorType::MERGE);

        explicit MergeOperator() = default;

        [[nodiscard]] core::shared::types::OperatorTypeId getOperatorTypeId() const override;

    protected:
        void validateInput() override;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MERGE_LOGICAL_MERGE_OPERATOR_HPP