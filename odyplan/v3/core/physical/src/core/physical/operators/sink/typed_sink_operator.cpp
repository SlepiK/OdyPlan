#include <core/physical/operators/sink/typed_sink_operator.h>

namespace sierra::odyplan::v3::core::physical::operators::sink {
    TypedSinkOperator::TypedSinkOperator(port::Port inputPort) {
        if (inputPort.empty()) {
            throw std::invalid_argument("TypedSinkOperator: inputPort must not be empty");
        }
        getInputPortsRef().push_back(std::move(inputPort));
    }
}