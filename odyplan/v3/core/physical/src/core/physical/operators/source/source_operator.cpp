#include <core/physical/operators/source/source_operator.h>

namespace sierra::odyplan::v3::core::physical::operators::source {
    bool ISourceOperator::hasInputPorts() const {
        return false;
    }

    bool ISourceOperator::hasOutputPorts() const {
        return true;
    }

    OperatorRole ISourceOperator::getRole() const {
        return OperatorRole::SOURCE;
    }
}  // namespace sierra::odyplan::v3::core::physical::operators::source