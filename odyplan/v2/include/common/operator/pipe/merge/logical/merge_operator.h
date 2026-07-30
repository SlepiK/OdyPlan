#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_MERGE_OPERATOR_LOGICAL_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_MERGE_OPERATOR_LOGICAL_H

#include <utility>
#include <vector>
#include "core/logical/operator/pipe/pipe_operator.h"

namespace sierra::odyplan::v2::common::opr::pipe::logical {
    class MergeOperator : public core::logical::opr::TypedPipeOperator {
    public:
        static constexpr core::common::identifier::OperatorTypeId TYPE_ID = std::to_underlying(
            core::common::identifier::Operator::MERGE);

        explicit MergeOperator() = default;

        [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
            return TYPE_ID;
        }

    protected:
        void validateInput() override {
            const auto &inputPortsRef = getInputPortsRef();
            const auto &outputPortRef = getOutputPortsRef().at(0);

            for (const auto &inputPortRef: inputPortsRef) {
                if (inputPortRef != outputPortRef) {
                    throw std::logic_error(
                        "MergeOperator::validate: all InputPorts and the first OutputPort need to be the same");
                }
            }
        }
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_MERGE_OPERATOR_LOGICAL_H