#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_SINKOPERATOR_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_SINKOPERATOR_H

#include "core/operator/operator.h"

namespace sierra::odyplan::v1::core::opr {
    class ISinkOperator : public virtual IOperator {
    public:
        ISinkOperator() = default;
        ~ISinkOperator() override = default;

        ISinkOperator(const ISinkOperator&) = delete;
        ISinkOperator& operator=(const ISinkOperator&) = delete;

        ISinkOperator(ISinkOperator&&) = delete;
        ISinkOperator& operator=(ISinkOperator&&) = delete;
    };
}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_SINKOPERATOR_H