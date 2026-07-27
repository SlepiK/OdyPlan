#ifndef SIERRA_ODYPLAN_V2_CORE_PHYSICAL_OPERATOR_SINK_SINKOPERATOR_H
#define SIERRA_ODYPLAN_V2_CORE_PHYSICAL_OPERATOR_SINK_SINKOPERATOR_H

#include "core/physical/operator/operator.h"
#include "../operator_ability.h"
#include "core/physical/port/port.h"

namespace sierra::odyplan::v2::core::physical::opr {
    // NOLINTNEXTLINE(misc-multiple-inheritance)
    class ISinkOperator : public IOperator, public IConsumer {
    public:
        [[nodiscard]] bool hasInputPorts() const override {
            return true;
        }

        [[nodiscard]] bool hasOutputPorts() const override {
            return false;
        }

        [[nodiscard]] OperatorRole getRole() const final {
            return OperatorRole::SINK;
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
}  // namespace sierra::odyplan::v2::core::physical::opr

#endif  // SIERRA_ODYPLAN_V2_CORE_PHYSICAL_OPERATOR_SINK_SINKOPERATOR_H