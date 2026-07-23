#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_PIPE_PIPEOPERATOR_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_PIPE_PIPEOPERATOR_H

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
            /**
             * 1. Get Reference to current InputPorts
             * 2. Verify that we have enough space in InputPorts or resize it by one field
             * 3. Set the InputPort at the select Port/Index
             * 4. Get Reference to the OutputPorts
             * 5. Get the first OutputPort and set it to the first InputPort
             *
             * Else an overwrite is needed. Example:
             *  outputPort.insert(
             *      outputPort.end(),
             *      std::make_move_iterator(input.begin()),
             *      std::make_move_iterator(input.end())
             *  );
             */

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
    };

    class TypedPipeOperator : public IPipeOperator {
    public:
        explicit TypedPipeOperator() = default;
    };
}  // namespace sierra::odyplan::v2::core::logical::opr

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_PIPE_PIPEOPERATOR_H