#include <core/logical/operators/source/source_operator.h>

namespace sierra::odyplan::v3::core::logical::operators::source {
    bool ISourceOperator::hasInputPorts() const {
        return false;
    }

    bool ISourceOperator::hasOutputPorts() const {
        return true;
    }
}  // namespace sierra::odyplan::v3::core::logical::operators::source