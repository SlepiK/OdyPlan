#include <core/physical/operators/operator.h>

namespace sierra::odyplan::v3::core::physical::operators {
    bool IOperator::isSegmentBreaker() const {
        return false;
    }

    const port::Ports& IOperator::getInputPorts() const {
        return mInputPorts;
    }

    const port::Ports& IOperator::getOutputPorts() const {
        return mOutputPorts;
    }

    void IOperator::declareState(state::StateRegistry& stateRegistry,
                                 llvm::Module& module,
                                 llvm::LLVMContext& context) {}

    shared::identifiers::OperatorId IOperator::getId() const {
        return mId;
    }

    void IOperator::setId(const shared::identifiers::OperatorId id) {
        mId = id;
    }

    port::Ports& IOperator::getInputPortsRef() {
        return mInputPorts;
    }

    port::Ports& IOperator::getOutputPortsRef() {
        return mOutputPorts;
    }
}  // namespace sierra::odyplan::v3::core::physical::operators