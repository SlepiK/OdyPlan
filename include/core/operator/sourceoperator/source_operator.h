#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_SOURCEOPERATOR_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_SOURCEOPERATOR_H

#include "core/operator/operator.h"

namespace sierra::odyplan::v1::core::opr {
    class ISourceOperator : public virtual IOperator {
    public:
        ISourceOperator() = default;
        ~ISourceOperator() override = default;

        ISourceOperator(const ISourceOperator&) = delete;
        ISourceOperator& operator=(const ISourceOperator&) = delete;

        ISourceOperator(ISourceOperator&&) = delete;
        ISourceOperator& operator=(ISourceOperator&&) = delete;
    };
}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_SOURCEOPERATOR_H