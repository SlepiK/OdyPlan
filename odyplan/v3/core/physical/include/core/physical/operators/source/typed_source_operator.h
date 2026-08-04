#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_SOURCE_TYPED_SOURCE_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_SOURCE_TYPED_SOURCE_OPERATOR_HPP

#include <core/physical/operators/source/source_operator.h>

namespace sierra::odyplan::v3::core::physical::operators::source {
    class TypedSourceOperator : public ISourceOperator {
    public:
        explicit TypedSourceOperator(port::Port outputPort);
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_SOURCE_TYPED_SOURCE_OPERATOR_HPP