#ifndef SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_PIPE_PIPE_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_PIPE_PIPE_OPERATOR_HPP

#include <core/logical/operators/bindable/bindable_operator.h>
#include <core/logical/operators/operator.h>
#include <core/logical/port/port.h>
#include <core/shared/identifiers/port_id.h>

namespace sierra::odyplan::v3::core::logical::operators::pipe {
    class IPipeOperator : public IOperator, public bindable::IBindableOperator {
    public:
        using PortId = shared::identifiers::PortId;
        using Port = port::Port;

        [[nodiscard]] bool hasInputPorts() const override;

        [[nodiscard]] bool hasOutputPorts() const override;

    protected:
        void bindInput(PortId portId, Port input) override;
    };
}  // namespace sierra::odyplan::v3::core::logical::operators::pipe

#endif  // SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_PIPE_PIPE_OPERATOR_HPP