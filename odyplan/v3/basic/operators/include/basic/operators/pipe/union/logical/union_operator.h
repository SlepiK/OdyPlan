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

        explicit UnionOperator(const bool assureOrder = true) : mAssureOrder(assureOrder) {
        }

        [[nodiscard]] core::shared::types::OperatorTypeId getOperatorTypeId() const override;

        [[nodiscard]] bool isOutOfOrder() const;

    protected:
        void validateInput() override;

    private:
        bool mAssureOrder; //true == IN_ORDER | false == OUT_OF_ORDER
    };
}

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_LOGICAL_UNION_OPERATOR_HPP