#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_SINK_TYPED_SINK_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_SINK_TYPED_SINK_OPERATOR_HPP

#include <core/physical/operators/sink/sink_operator.h>

namespace sierra::odyplan::v3::core::physical::operators::sink {
    class TypedSinkOperator : public ISinkOperator {
    public:
        explicit TypedSinkOperator(port::Port inputPort);
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_SINK_TYPED_SINK_OPERATOR_HPP