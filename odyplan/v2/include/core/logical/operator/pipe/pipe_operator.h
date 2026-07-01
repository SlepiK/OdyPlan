#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_PIPE_PIPEOPERATOR_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_PIPE_PIPEOPERATOR_H

#include <algorithm>

#include "core/logical/operator/operator.h"
#include "core/logical/port/port.h"

namespace sierra::odyplan::v2::core::logical::opr {
    class IPipeOperator : public IOperator {
    public:
        [[nodiscard]] bool hasInputPorts() const override {
            return true;
        }

        [[nodiscard]] bool hasOutputPorts() const override {
            return true;
        }
    };

    class TypedPipeOperator : public IPipeOperator {
    public:
        explicit TypedPipeOperator(port::Ports inputPorts, port::Ports outputPorts) {
            if (inputPorts.empty()) {
                throw std::invalid_argument("TypedPipeOperator: inputPorts must not be empty");
            }
            if (outputPorts.empty()) {
                throw std::invalid_argument("TypedPipeOperator: outputPorts must not be empty");
            }
            if (std::ranges::any_of(inputPorts, [](const port::Port& port) {return port.empty();})) {
                throw std::invalid_argument("TypedPipeOperator: no empty input port allowed");
            }
            if (std::ranges::any_of(outputPorts, [](const port::Port& port) {return port.empty();})) {
                throw std::invalid_argument("TypedPipeOperator: no empty output port allowed");
            }
            getInputPortsRef() = std::move(inputPorts);
            getOutputPortsRef() = std::move(outputPorts);
        }
    };
}  // namespace sierra::odyplan::v2::core::logical::opr

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_PIPE_PIPEOPERATOR_H