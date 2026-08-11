#ifndef SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_SINK_TYPED_SINK_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_SINK_TYPED_SINK_OPERATOR_HPP

#include <core/logical/operators/sink/sink_operator.h>

namespace sierra::odyplan::v3::core::logical::operators::sink {
    class TypedSinkOperator : public ISinkOperator {
    public:
        explicit TypedSinkOperator() = default;
    };
}  // namespace sierra::odyplan::v3::core::logical::operators::sink

#endif  // SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_SINK_TYPED_SINK_OPERATOR_HPP