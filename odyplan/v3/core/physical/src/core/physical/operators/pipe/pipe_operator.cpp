#include <core/physical/operators/pipe/pipe_operator.h>

namespace sierra::odyplan::v3::core::physical::operators::pipe {
    bool IPipeOperator::hasInputPorts() const {
        return true;
    }

    bool IPipeOperator::hasOutputPorts() const {
        return true;
    }

    OperatorRole IPipeOperator::getRole() const {
        return OperatorRole::PIPE;
    }
}  // namespace sierra::odyplan::v3::core::physical::operators::pipe
