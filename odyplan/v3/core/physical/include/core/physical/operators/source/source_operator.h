#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_SOURCE_SOURCE_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_SOURCE_SOURCE_OPERATOR_HPP

#include <core/physical/operators/operator.h>
#include <core/physical/flow/flow_producer.h>

namespace sierra::odyplan::v3::core::physical::operators::source {
    class ISourceOperator : public IOperator, public flow::IProducer {
    public:
        [[nodiscard]] bool hasInputPorts() const override;

        [[nodiscard]] bool hasOutputPorts() const override;

        [[nodiscard]] OperatorRole getRole() const final;
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_SOURCE_SOURCE_OPERATOR_HPP