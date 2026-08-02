#ifndef SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_SOURCES_TYPED_SOURCE_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_SOURCES_TYPED_SOURCE_OPERATOR_HPP

#include <core/logical/operators/sources/source_operator.h>
#include <core/logical/ports/port.h>

namespace sierra::odyplan::v3::core::logical::operators::sources {
    class TypedSourceOperator : public ISourceOperator {
    public:
        using Port = ports::Port;

        explicit TypedSourceOperator(Port outputPort);
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_SOURCES_TYPED_SOURCE_OPERATOR_HPP