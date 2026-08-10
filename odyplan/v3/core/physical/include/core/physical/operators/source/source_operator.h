#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_SOURCE_SOURCE_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_SOURCE_SOURCE_OPERATOR_HPP

#include <core/physical/flow/flow_producer.h>
#include <core/physical/operators/operator.h>

namespace sierra::odyplan::v3::core::physical::operators::source {
    class ISourceOperator : public IOperator, public flow::IProducer {
    public:
        [[nodiscard]] bool hasInputPorts() const override;

        [[nodiscard]] bool hasOutputPorts() const override;

        [[nodiscard]] OperatorRole getRole() const final;
    };
}  // namespace sierra::odyplan::v3::core::physical::operators::source

#endif  // SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_SOURCE_SOURCE_OPERATOR_HPP