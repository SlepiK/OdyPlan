#ifndef SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_IOPERATOR_HPP
#define SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_IOPERATOR_HPP

#include <core/shared/types/operator_type_id.h>
#include <core/logical/port/port.h>

namespace sierra::odyplan::v3::core::builder {
    class PlanBuilder;
}

namespace sierra::odyplan::v3::core::logical::plans {
    class Plan;
}

namespace sierra::odyplan::v3::core::logical::operators {
    class IOperator {
        friend class plans::Plan;
        friend class builder::PlanBuilder;
    public:
        using OperatorTypeId = shared::types::OperatorTypeId;
        using Ports = port::Ports;

        IOperator() = default;

        virtual ~IOperator() = default;

        IOperator(const IOperator&) = delete;

        IOperator& operator=(const IOperator&) = delete;

        IOperator(IOperator&&) = delete;

        IOperator& operator=(IOperator&&) = delete;

        [[nodiscard]] virtual bool hasInputPorts() const = 0;

        [[nodiscard]] virtual bool hasOutputPorts() const = 0;

        [[nodiscard]] const Ports& getInputPorts() const;

        [[nodiscard]] const Ports& getOutputPorts() const;

        [[nodiscard]] virtual OperatorTypeId getOperatorTypeId() const = 0;

    private:
        Ports mInputPorts;
        Ports mOutputPorts;

    protected:
        Ports& getInputPortsRef();

        Ports& getOutputPortsRef();

        virtual void validate() const {
        }
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_IOPERATOR_HPP