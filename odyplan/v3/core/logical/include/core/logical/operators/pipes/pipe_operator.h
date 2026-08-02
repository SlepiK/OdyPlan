#ifndef SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_PIPES_PIPE_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_PIPES_PIPE_OPERATOR_HPP

#include <core/shared/identifiers/port_id.h>
#include <core/logical/ports/port.h>

#include <core/logical/operators/operator.h>
#include <core/logical/operators/bindables/bindable_operator.h>

namespace sierra::odyplan::v3::core::logical::operators::pipes {
    class IPipeOperator : public IOperator, public bindables::IBindableOperator {
    public:
        using PortId = shared::identifiers::PortId;
        using Port = ports::Port;

        [[nodiscard]] bool hasInputPorts() const override;

        [[nodiscard]] bool hasOutputPorts() const override;

    protected:
        void bindInput(PortId portId, Port input) override;
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_PIPES_PIPE_OPERATOR_HPP