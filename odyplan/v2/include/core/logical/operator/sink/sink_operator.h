#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_SINK_SINKOPERATOR_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_SINK_SINKOPERATOR_H

#include "core/logical/operator/operator.h"
#include "core/logical/port/port.h"

namespace sierra::odyplan::v2::core::logical::opr {
    class ISinkOperator : public IOperator {
    public:
        bool hasInputPorts() const override {
            return true;
        }

        bool hasOutputPorts() const override {
            return false;
        }
    };

    class TypedSinkOperator : public ISinkOperator {
    public:
        explicit TypedSinkOperator(port::Port inputPort) {
            if (inputPort.empty()) {
                throw std::invalid_argument("TypedSinkOperator: inputPort must not be empty");
            }
            getInputPortsRef().push_back(std::move(inputPort));
        }
    };
}  // namespace sierra::odyplan::v2::core::logical::opr

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_SINK_SINKOPERATOR_H