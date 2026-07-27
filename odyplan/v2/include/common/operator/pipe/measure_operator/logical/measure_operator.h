#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_MEASURE_OPERATOR_LOGICAL_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_MEASURE_OPERATOR_LOGICAL_H

#include <utility>
#include "core/logical/operator/pipe/pipe_operator.h"

namespace sierra::odyplan::v2::common::opr::pipe::logical {
    class MeasureOperator : public core::logical::opr::TypedPipeOperator {
    public:
        static constexpr core::common::identifier::OperatorTypeId TYPE_ID = std::to_underlying(
            core::common::identifier::Operator::MEASURE);

        explicit MeasureOperator(std::string label)
            : mLabel(std::move(label)) {
        }

        [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
            return TYPE_ID;
        }

        [[nodiscard]] const std::string &getLabel() const {
            return mLabel;
        }

    protected:
        void validate() override {
        }

    private:
        std::string mLabel;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_MEASURE_OPERATOR_LOGICAL_H