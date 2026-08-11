#include <core/logical/operators/operator.h>

namespace sierra::odyplan::v3::core::logical::operators {
    const IOperator::Ports& IOperator::getInputPorts() const {
        return mInputPorts;
    }

    const IOperator::Ports& IOperator::getOutputPorts() const {
        return mOutputPorts;
    }

    IOperator::Ports& IOperator::getInputPortsRef() {
        return mInputPorts;
    }

    IOperator::Ports& IOperator::getOutputPortsRef() {
        return mOutputPorts;
    }
}  // namespace sierra::odyplan::v3::core::logical::operators