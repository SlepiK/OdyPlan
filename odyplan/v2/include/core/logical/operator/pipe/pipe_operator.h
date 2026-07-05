#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_PIPE_PIPEOPERATOR_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_PIPE_PIPEOPERATOR_H

#include <algorithm>

#include "core/logical/operator/operator.h"
#include "core/logical/port/port.h"

namespace sierra::odyplan::v2::core::logical::opr {
    class IPipeOperator : public IOperator, public IBindable {
    public:
        [[nodiscard]] bool hasInputPorts() const override {
            return true;
        }

        [[nodiscard]] bool hasOutputPorts() const override {
            return true;
        }

    protected:
        void bindInput(const common::identifier::PortId portId, port::Port input) override {
            auto& inputPorts = getInputPortsRef();

            if (inputPorts.size() <= portId) {
                inputPorts.resize(portId + 1);
            }

            inputPorts.at(portId) = input;

            auto& outputPorts = getOutputPortsRef();

            if (outputPorts.empty()) {
                outputPorts.resize(1);
            }

            auto& outputPort = outputPorts.at(0);
            outputPort.insert(
                outputPort.end(),
                std::make_move_iterator(input.begin()),
                std::make_move_iterator(input.end())
                );

        }
    };

    class TypedPipeOperator : public IPipeOperator {
    public:
        explicit TypedPipeOperator() = default;
    };
}  // namespace sierra::odyplan::v2::core::logical::opr

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_PIPE_PIPEOPERATOR_H