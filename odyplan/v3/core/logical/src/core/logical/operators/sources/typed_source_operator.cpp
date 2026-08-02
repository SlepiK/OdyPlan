#include <stdexcept>
#include <core/logical/operators/sources/typed_source_operator.h>

namespace sierra::odyplan::v3::core::logical::operators::sources {
    TypedSourceOperator::TypedSourceOperator(Port outputPort) {
        if (outputPort.empty()) {
            throw std::invalid_argument("TypedSourceOperator: outputPort must not be empty");
        }
        getOutputPortsRef().push_back(std::move(outputPort));
    }
}