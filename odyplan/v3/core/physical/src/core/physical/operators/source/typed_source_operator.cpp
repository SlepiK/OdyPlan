#include <core/physical/operators/source/typed_source_operator.h>

namespace sierra::odyplan::v3::core::physical::operators::source {
    TypedSourceOperator::TypedSourceOperator(port::Port outputPort) {
        if (outputPort.empty()) {
            throw std::invalid_argument("TypedSourceOperator: outputPort must not be empty");
        }
        getOutputPortsRef().push_back(std::move(outputPort));
    }
}