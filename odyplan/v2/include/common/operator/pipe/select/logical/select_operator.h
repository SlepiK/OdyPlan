#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_SELECT_OPERATOR_LOGICAL_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_SELECT_OPERATOR_LOGICAL_H

#include <utility>

#include "core/logical/operator/pipe/pipe_operator.h"
#include "core/common/expression/expression.h"

namespace sierra::odyplan::v2::common::opr::pipe::logical {
    class SelectOperator : public core::logical::opr::TypedPipeOperator {
    public:
        static constexpr core::common::identifier::OperatorTypeId TYPE_ID = std::to_underlying(
            core::common::identifier::Operator::SELECT);

        explicit SelectOperator(std::unique_ptr<core::common::expression::IExpression> predicate)
            : mPredicate(std::move(predicate)) {
        }

        [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
            return TYPE_ID;
        }

        [[nodiscard]] const core::common::expression::IExpression &getPredicate() const {
            return *mPredicate;
        }

    protected:
        void validateInput() override {
            if (mPredicate == nullptr) {
                throw std::invalid_argument("SelectOperator: predicate must be non-null");
            }
        }

    private:
        std::unique_ptr<core::common::expression::IExpression> mPredicate;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_SELECT_OPERATOR_LOGICAL_H