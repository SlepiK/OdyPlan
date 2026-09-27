#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_LOGICAL_JOIN_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_LOGICAL_JOIN_OPERATOR_HPP

#include <basic/expression/logical/expression.h>
#include <basic/expression/physical/expression.h>
#include <basic/expression/translate/logical_to_physical_translator.h>
#include <basic/operators/pipe/join/logical/details/join_config.h>
#include <core/logical/operators/pipe/typed_pipe_operator.h>
#include <utility>

namespace sierra::odyplan::v3::basic::operators::pipe::join::logical {
    class JoinOperator : public core::logical::operators::pipe::TypedPipeOperator {
    public:
        static constexpr core::shared::types::OperatorTypeId TYPE_ID =
            std::to_underlying(core::shared::types::BuiltinOperatorType::JOIN);

        explicit JoinOperator(details::JoinConfig config,
                              std::unique_ptr<basic::expression::logical::IExpression> predicate = nullptr);

        [[nodiscard]] core::shared::types::OperatorTypeId getOperatorTypeId() const override;

        [[nodiscard]] const details::JoinConfig& getConfig() const;

        [[nodiscard]] std::unique_ptr<basic::expression::physical::IExpression> getPredicate();

    protected:
        void bindInput(core::shared::identifiers::PortId portId,
                       core::logical::port::Port input) override;

        void validateInput() override;

        void validate() const override;

    private:
        details::JoinConfig mConfig;
        const std::unique_ptr<basic::expression::logical::IExpression> mLPredicate;
        std::unique_ptr<basic::expression::physical::IExpression> mPPredicate;
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::logical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_LOGICAL_JOIN_OPERATOR_HPP