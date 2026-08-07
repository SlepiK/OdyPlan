#ifndef SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_SOURCE_TYPED_SOURCE_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_SOURCE_TYPED_SOURCE_OPERATOR_HPP

#include <core/logical/operators/source/source_operator.h>
#include <core/logical/port/port.h>

namespace sierra::odyplan::v3::core::logical::operators::source {
    class TypedSourceOperator : public ISourceOperator {
    public:
        using Port = port::Port;

        explicit TypedSourceOperator(Port outputPort);
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_SOURCE_TYPED_SOURCE_OPERATOR_HPP