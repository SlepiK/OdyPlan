#ifndef SIERRA_ODYPLAN_V2_EXAMPLE_LOGICALPLAN_NUMBERSTDOUTSINKOPERATOR_H
#define SIERRA_ODYPLAN_V2_EXAMPLE_LOGICALPLAN_NUMBERSTDOUTSINKOPERATOR_H

#include "core/logical/operator/sink/sink_operator.h"
#include "const_number_source_operator.h"

namespace sierra::odyplan::v2::example::logical_plan {
    class NumberStdoutSinkOperator final : public v2::core::logical::opr::TypedSinkOperator {
    public:
        NumberStdoutSinkOperator() : TypedSinkOperator(createIntPort()) {
        }
    };
} // namespace sierra::odyplan::v2::example::logical_plan

#endif //SIERRA_ODYPLAN_V2_EXAMPLE_LOGICALPLAN_NUMBERSTDOUTSINKOPERATOR_H