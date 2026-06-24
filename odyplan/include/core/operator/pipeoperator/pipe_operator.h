#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_PIPEOPERATOR_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_PIPEOPERATOR_H

#include "core/operator/operator.h"

namespace sierra::odyplan::v1::core::opr {
    class IPipeOperator : public virtual IOperator {
    public:
        IPipeOperator() = default;
        ~IPipeOperator() override = default;

        IPipeOperator(const IPipeOperator&) = delete;
        IPipeOperator& operator=(const IPipeOperator&) = delete;

        IPipeOperator(IPipeOperator&&) = delete;
        IPipeOperator& operator=(IPipeOperator&&) = delete;
    };
}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_PIPEOPERATOR_H