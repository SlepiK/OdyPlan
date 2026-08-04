#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_OPERATOR_HPP

#include <core/shared/identifiers/operator_id.h>
#include <core/physical/operators/operator_role.h>
#include <core/physical/state/state_registry.h>
#include <core/physical/port/port.h>

namespace sierra::odyplan::v3::core::physical::operators {
    class IOperator {
    public:
        using Ports = port::Ports;

        IOperator() = default;

        virtual ~IOperator() = default;

        IOperator(const IOperator&) = delete;

        IOperator& operator=(const IOperator&) = delete;

        IOperator(IOperator&&) = delete;

        IOperator& operator=(IOperator&&) = delete;

        [[nodiscard]] virtual bool hasInputPorts() const = 0;

        [[nodiscard]] virtual bool hasOutputPorts() const = 0;

        [[nodiscard]] virtual bool isSegmentBreaker() const;

        [[nodiscard]] const port::Ports& getInputPorts() const;

        [[nodiscard]] const port::Ports& getOutputPorts() const;

        [[nodiscard]] virtual OperatorRole getRole() const = 0;

        virtual void declareState(state::StateRegistry& stateRegistry, llvm::Module& module, llvm::LLVMContext& context);

        [[nodiscard]] shared::identifiers::OperatorId getId() const;

        void setId(shared::identifiers::OperatorId id);

    private:
        Ports mInputPorts;
        Ports mOutputPorts;
        shared::identifiers::OperatorId mId = 0;

    protected:
        Ports& getInputPortsRef();

        Ports& getOutputPortsRef();
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_OPERATOR_HPP