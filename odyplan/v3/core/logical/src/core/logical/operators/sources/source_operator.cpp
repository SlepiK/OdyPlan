#include <core/logical/operators/sources/source_operator.h>

namespace sierra::odyplan::v3::core::logical::operators::sources {
    bool ISourceOperator::hasInputPorts() const {
        return false;
    }

    bool ISourceOperator::hasOutputPorts() const {
        return true;
    }
}