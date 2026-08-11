#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_SELECT_LOGICAL_SELECT_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_SELECT_LOGICAL_SELECT_OPERATOR_HPP

#include <basic/expression/expression.h>
#include <core/logical/operators/pipe/typed_pipe_operator.h>
#include <memory>
#include <utility>

namespace sierra::odyplan::v3::basic::operators::pipe::select::logical {
    class SelectOperator : public core::logical::operators::pipe::TypedPipeOperator {
    public:
        static constexpr core::shared::types::OperatorTypeId TYPE_ID =
            std::to_underlying(core::shared::types::BuiltinOperatorType::SELECT);

        explicit SelectOperator(std::unique_ptr<basic::expression::IExpression> predicate);

        [[nodiscard]] core::shared::types::OperatorTypeId getOperatorTypeId() const override;

        [[nodiscard]] const basic::expression::IExpression& getPredicate() const;

    protected:
        void validateInput() override;

    private:
        std::unique_ptr<basic::expression::IExpression> mPredicate;
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::select::logical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_SELECT_LOGICAL_SELECT_OPERATOR_HPP