#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_PIPE_PIPE_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_PIPE_PIPE_OPERATOR_HPP

#include <core/physical/operators/operator.h>
#include <core/physical/flow/flow_consumer.h>

namespace sierra::odyplan::v3::core::physical::operators::pipe {
    class IPipeOperator : public IOperator, public flow::IConsumer {
    public:
        [[nodiscard]] bool hasInputPorts() const override;

        [[nodiscard]] bool hasOutputPorts() const override;

        [[nodiscard]] OperatorRole getRole() const final;
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_PIPE_PIPE_OPERATOR_HPP