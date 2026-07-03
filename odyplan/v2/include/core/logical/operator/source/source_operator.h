#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_SOURCE_SOURCEOPERATOR_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_SOURCE_SOURCEOPERATOR_H

#include "core/logical/operator/operator.h"
#include "core/logical/port/port.h"

namespace sierra::odyplan::v2::core::logical::opr {
    class ISourceOperator : public IOperator {
    public:
        [[nodiscard]] bool hasInputPorts() const override {
            return false;
        }

        [[nodiscard]] bool hasOutputPorts() const override {
            return true;
        }
    };

    class TypedSourceOperator : public ISourceOperator {
    public:
        explicit TypedSourceOperator(port::Port outputPort) {
            if (outputPort.empty()) {
                throw std::invalid_argument("TypedSourceOperator: outputPort must not be empty");
            }
            getOutputPortsRef().push_back(std::move(outputPort));
        }
    };

}  // namespace sierra::odyplan::v2::core::logical::opr

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_SOURCE_SOURCEOPERATOR_H