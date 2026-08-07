#include <core/physical/operators/sink/sink_operator.h>

namespace sierra::odyplan::v3::core::physical::operators::sink {
    bool ISinkOperator::hasInputPorts() const {
        return true;
    }

    bool ISinkOperator::hasOutputPorts() const {
        return false;
    }

    OperatorRole ISinkOperator::getRole() const {
        return OperatorRole::SINK;
    }
}