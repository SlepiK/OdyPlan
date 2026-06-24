#ifndef SIERRA_ODYPLAN_V1_EXAMPLE_NUMBERSTREAM_CONSTNUMBERSOURCEOPERATOR_H
#define SIERRA_ODYPLAN_V1_EXAMPLE_NUMBERSTREAM_CONSTNUMBERSOURCEOPERATOR_H

#include "core/operator/sourceoperator/typed_source_operator.h"
#include "core/tuple/tuple.h"
#include <cstdint>

namespace sierra::odyplan::v1::example::number_stream {
    enum class NumberTupleField : std::uint8_t {
        NUMBER
    };

    using NumberTuple = core::tuple::TupleT<core::tuple::TupleFieldT<int, NumberTupleField::NUMBER>>;

    class ConstNumberSourceOperator final : public core::opr::TypedSourceOperator<NumberTuple> {
    public:
        bool produce() override {
            emit(NumberTuple{1});
            return true;
        }
    };
}

#endif //SIERRA_ODYPLAN_V1_EXAMPLE_NUMBERSTREAM_CONSTNUMBERSOURCEOPERATOR_H