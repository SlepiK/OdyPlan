#include <core/logical/operators/pipe/pipe_operator.h>

namespace sierra::odyplan::v3::core::logical::operators::pipe {
    bool IPipeOperator::hasInputPorts() const {
        return true;
    }

    bool IPipeOperator::hasOutputPorts() const {
        return true;
    }

    void IPipeOperator::bindInput(PortId portId, Port input) {
        auto& inputPorts = getInputPortsRef();

        if (inputPorts.size() <= portId) {
            inputPorts.resize(portId + 1);
        }

        inputPorts.at(portId) = input;

        auto& outputPorts = getOutputPortsRef();

        if (outputPorts.empty()) {
            outputPorts.resize(1);
        }

        if (auto& outputPort = outputPorts.at(0); outputPort.empty()) {
            outputPort = std::move(input);
        }
    }
}