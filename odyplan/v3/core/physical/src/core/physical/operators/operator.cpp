#include <core/physical/operators/operator.h>

namespace sierra::odyplan::v3::core::physical::operators {
    [[nodiscard]] virtual bool isSegmentBreaker() const;

    [[nodiscard]] const port::Ports& getInputPorts() const;

    [[nodiscard]] const port::Ports& getOutputPorts() const;

    virtual void declareState(state::StateRegistry& stateRegistry, llvm::Module& module, llvm::LLVMContext& context);

    [[nodiscard]] common::identifier::OperatorId getId() const;

    void setId(const common::identifier::OperatorId id);

    port::Ports& getInputPortsRef();

    port::Ports& getOutputPortsRef();
}