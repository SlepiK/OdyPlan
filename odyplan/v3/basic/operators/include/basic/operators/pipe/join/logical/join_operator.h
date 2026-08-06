#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_LOGICAL_JOIN_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_LOGICAL_JOIN_OPERATOR_HPP

#include <utility>

#include <basic/operators/pipe/join/logical/details/join_config.h>
#include <core/logical/operators/pipe/typed_pipe_operator.h>
#include <basic/expression/expression.h>

namespace sierra::odyplan::v3::basic::operators::pipe::join::logical {
    class JoinOperator : public core::logical::operators::pipe::TypedPipeOperator {
    public:
        static constexpr core::shared::types::OperatorTypeId TYPE_ID = std::to_underlying(
            core::shared::types::BuiltinOperatorType::JOIN);

        explicit JoinOperator(const details::JoinConfig config,
                                      const basic::expression::IExpression* predicate = nullptr);

        [[nodiscard]] core::shared::types::OperatorTypeId getOperatorTypeId() const override;

        [[nodiscard]] const details::JoinConfig& getConfig() const;

        [[nodiscard]] const basic::expression::IExpression* getPredicate() const;

    protected:
        void bindInput(const core::shared::identifiers::PortId portId, const core::logical::port::Port input) override;

        void validateInput() override;

        void validate() const override;

    private:
        details::JoinConfig mConfig;
        const basic::expression::IExpression* mPredicate;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_LOGICAL_JOIN_OPERATOR_HPP