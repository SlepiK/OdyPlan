#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_SINK_SINKOPERATOR_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_SINK_SINKOPERATOR_H

#include "core/logical/operator/operator.h"
#include "core/logical/port/port.h"

namespace sierra::odyplan::v2::core::logical::opr {
    class ISinkOperator : public IOperator, public IBindable {
    public:
        [[nodiscard]] bool hasInputPorts() const override {
            return true;
        }

        [[nodiscard]] bool hasOutputPorts() const override {
            return false;
        }

    protected:
        void bindInput(const common::identifier::PortId portId, port::Port input) override {
            auto& inputPorts = getInputPortsRef();

            if (inputPorts.size() == 1) {
                throw std::runtime_error("ISinkOperator: cant bind more than one input");
            }

            if (inputPorts.size() <= portId) {
                inputPorts.resize(portId + 1);
            }

            inputPorts.at(portId) = std::move(input);
        }

    };

    class TypedSinkOperator : public ISinkOperator {
    public:
        explicit TypedSinkOperator() = default;
    };
}  // namespace sierra::odyplan::v2::core::logical::opr

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_SINK_SINKOPERATOR_H