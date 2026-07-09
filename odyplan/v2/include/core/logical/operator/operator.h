#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_OPERATOR_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_OPERATOR_H

#include "core/common/identifier/operator.h"
#include "core/logical/port/port.h"

namespace sierra::odyplan::v2::core::logical::plan {
    class Plan;
}

namespace sierra::odyplan::v2::core::logical::opr {
    class IBindable {
        friend class plan::Plan;
    public:
        IBindable() = default;
        virtual ~IBindable() = default;

        IBindable(const IBindable&) = delete;
        IBindable& operator=(const IBindable&) = delete;

        IBindable(IBindable&&) = delete;
        IBindable& operator=(IBindable&&) = delete;
    protected:
        virtual void bindInput(common::identifier::PortId portId, port::Port input) = 0;
        virtual void validate() = 0;
    };

    class IOperator {
        friend class plan::Plan;
    public:
        IOperator() = default;
        virtual ~IOperator() = default;

        IOperator(const IOperator&) = delete;
        IOperator& operator=(const IOperator&) = delete;

        IOperator(IOperator&&) = delete;
        IOperator& operator=(IOperator&&) = delete;

        [[nodiscard]] virtual bool hasInputPorts() const = 0;
        [[nodiscard]] virtual bool hasOutputPorts() const = 0;

        [[nodiscard]] const port::Ports& getInputPorts() const {
            return mInputPorts;
        };

        [[nodiscard]] const port::Ports& getOutputPorts() const {
            return mOutputPorts;
        }

        [[nodiscard]] virtual common::identifier::OperatorTypeId getOperatorTypeId() const = 0;

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

}  // namespace sierra::odyplan::v2::core::logical::opr

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_OPERATOR_H