#include <core/logical/operators/source/typed_source_operator.h>
#include <stdexcept>

namespace sierra::odyplan::v3::core::logical::operators::source {
    TypedSourceOperator::TypedSourceOperator(Port outputPort) {
        if (outputPort.empty()) {
            throw std::invalid_argument("TypedSourceOperator: outputPort must not be empty");
        }
        getOutputPortsRef().push_back(std::move(outputPort));
    }
}  // namespace sierra::odyplan::v3::core::logical::operators::source