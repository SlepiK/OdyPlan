#include <core/physical/operators/pipe/typed_pipe_operator.h>

namespace sierra::odyplan::v3::core::physical::operators::pipe {
    TypedPipeOperator::TypedPipeOperator(port::Ports inputPorts, port::Ports outputPorts) {
        if (inputPorts.empty()) {
            throw std::invalid_argument("TypedPipeOperator: inputPorts must not be empty");
        }
        if (outputPorts.empty()) {
            throw std::invalid_argument("TypedPipeOperator: outputPorts must not be empty");
        }
        if (std::ranges::any_of(inputPorts, [](const port::Port& port) { return port.empty(); })) {
            throw std::invalid_argument("TypedPipeOperator: no empty input port allowed");
        }
        if (std::ranges::any_of(outputPorts, [](const port::Port& port) { return port.empty(); })) {
            throw std::invalid_argument("TypedPipeOperator: no empty output port allowed");
        }
        getInputPortsRef() = std::move(inputPorts);
        getOutputPortsRef() = std::move(outputPorts);
    }
}  // namespace sierra::odyplan::v3::core::physical::operators::pipe
