#ifndef SIERRA_ODYPLAN_V2_CORE_PHYSICAL_OPERATOR_OPERATOR_H
#define SIERRA_ODYPLAN_V2_CORE_PHYSICAL_OPERATOR_OPERATOR_H

#include "core/physical/port/port.h"

namespace sierra::odyplan::v2::core::physical::opr {
    enum class OperatorRole : std::uint8_t {
        SOURCE,
        PIPE,
        SINK
    };

    class IOperator {
    public:
        IOperator() = default;
        virtual ~IOperator() = default;

        IOperator(const IOperator&) = delete;
        IOperator& operator=(const IOperator&) = delete;

        IOperator(IOperator&&) = delete;
        IOperator& operator=(IOperator&&) = delete;

        [[nodiscard]] virtual bool hasInputPorts() const = 0;
        [[nodiscard]] virtual bool hasOutputPorts() const = 0;

        [[nodiscard]] virtual bool isSegmentBreaker() const {
            return false;
        }

        [[nodiscard]] const port::Ports& getInputPorts() const {
            return mInputPorts;
        }

        [[nodiscard]] const port::Ports& getOutputPorts() const {
            return mOutputPorts;
        }

        [[nodiscard]] virtual OperatorRole getRole() const = 0;

    private:
        port::Ports mInputPorts;
        port::Ports mOutputPorts;

    protected:
        port::Ports& getInputPortsRef() {
            return mInputPorts;
        }

        port::Ports& getOutputPortsRef() {
            return mOutputPorts;
        }
    };

}  // namespace sierra::odyplan::v2::core::physical::opr

#endif  // SIERRA_ODYPLAN_V2_CORE_PHYSICAL_OPERATOR_OPERATOR_H