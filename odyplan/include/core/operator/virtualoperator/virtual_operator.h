#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_VIRTUALOPERATOR_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_VIRTUALOPERATOR_H

#include "core/operator/operator.h"

namespace sierra::odyplan::v1::core::opr {
    class IVirtualOperator : public virtual IOperator {
    public:
        IVirtualOperator() = default;
        ~IVirtualOperator() override = default;

        IVirtualOperator(const IVirtualOperator&) = delete;
        IVirtualOperator& operator=(const IVirtualOperator&) = delete;

        IVirtualOperator(IVirtualOperator&&) = delete;
        IVirtualOperator& operator=(IVirtualOperator&&) = delete;
    };
}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_VIRTUALOPERATOR_H