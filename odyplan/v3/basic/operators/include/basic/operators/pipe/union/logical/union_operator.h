#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_LOGICAL_UNION_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_LOGICAL_UNION_OPERATOR_HPP

#include <core/logical/operators/pipe/typed_pipe_operator.h>
#include <utility>
#include <vector>

namespace sierra::odyplan::v3::basic::operators::pipe::union_::logical {// NOLINT(readability-identifier-naming)
    class UnionOperator : public core::logical::operators::pipe::TypedPipeOperator {
    public:
        static constexpr core::shared::types::OperatorTypeId TYPE_ID =
            std::to_underlying(core::shared::types::BuiltinOperatorType::UNION);

        explicit UnionOperator(const bool order = true) : mOutOfOrder(order) {
        }

        [[nodiscard]] core::shared::types::OperatorTypeId getOperatorTypeId() const override;

        [[nodiscard]] bool getOutOfOrder() const;

    protected:
        void validateInput() override;

    private:
        bool mOutOfOrder;
    };
}

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_LOGICAL_UNION_OPERATOR_HPP