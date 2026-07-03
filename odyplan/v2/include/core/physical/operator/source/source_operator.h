#ifndef SIERRA_ODYPLAN_V2_CORE_PHYSICAL_OPERATOR_SOURCE_SOURCEOPERATOR_H
#define SIERRA_ODYPLAN_V2_CORE_PHYSICAL_OPERATOR_SOURCE_SOURCEOPERATOR_H

#include "core/physical/operator/operator.h"
#include "../operator_ability.h"
#include "core/physical/port/port.h"

namespace sierra::odyplan::v2::core::physical::opr {
    class ISourceOperator : public IOperator, public IProducer {
    public:
        [[nodiscard]] bool hasInputPorts() const override {
            return false;
        }

        [[nodiscard]] bool hasOutputPorts() const override {
            return true;
        }

        [[nodiscard]] OperatorRole getRole() const final {
            return OperatorRole::SOURCE;
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

}  // namespace sierra::odyplan::v2::core::physical::opr

#endif  // SIERRA_ODYPLAN_V2_CORE_PHYSICAL_OPERATOR_SOURCE_SOURCEOPERATOR_H