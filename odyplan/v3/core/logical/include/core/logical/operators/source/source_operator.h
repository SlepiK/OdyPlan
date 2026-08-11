#ifndef SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_SOURCE_SOURCE_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_SOURCE_SOURCE_OPERATOR_HPP

#include <core/logical/operators/operator.h>

namespace sierra::odyplan::v3::core::logical::operators::source {
    class ISourceOperator : public IOperator {
    public:
        [[nodiscard]] bool hasInputPorts() const override;

        [[nodiscard]] bool hasOutputPorts() const override;
    };
}  // namespace sierra::odyplan::v3::core::logical::operators::source

#endif  // SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_SOURCE_SOURCE_OPERATOR_HPP