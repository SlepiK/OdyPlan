#ifndef SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_SINKS_TYPED_SINK_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_SINKS_TYPED_SINK_OPERATOR_HPP

#include <core/logical/operators/sinks/sink_operator.h>

namespace sierra::odyplan::v3::core::logical::operators::sinks {
    class TypedSinkOperator : public ISinkOperator {
    public:
        explicit TypedSinkOperator() = default;
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_SINKS_TYPED_SINK_OPERATOR_HPP