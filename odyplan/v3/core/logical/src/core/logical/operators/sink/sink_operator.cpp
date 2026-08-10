#include <core/logical/operators/sink/sink_operator.h>
#include <stdexcept>

namespace sierra::odyplan::v3::core::logical::operators::sink {
    bool ISinkOperator::hasInputPorts() const {
        return true;
    }

    bool ISinkOperator::hasOutputPorts() const {
        return false;
    }

    void ISinkOperator::bindInput(const PortId portId, Port input) {
        auto& inputPorts = getInputPortsRef();

        if (inputPorts.size() == 1) {
            throw std::runtime_error("ISinkOperator: cant bind more than one input");
        }

        if (inputPorts.size() <= portId) {
            inputPorts.resize(portId + 1);
        }

        inputPorts.at(portId) = std::move(input);
    }
}  // namespace sierra::odyplan::v3::core::logical::operators::sink