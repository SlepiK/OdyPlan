#ifndef SIERRA_ODYPLAN_V1_TEST_OPERATOR_SOURCE_CONSTNUMBERSOURCEOPERATOR_H
#define SIERRA_ODYPLAN_V1_TEST_OPERATOR_SOURCE_CONSTNUMBERSOURCEOPERATOR_H

#include "core/operator/sourceoperator/typed_source_operator.h"
#include "core/tuple/tuple.h"
#include <string_view>
#include <cstdint>

namespace sierra::odyplan::v1::test::opr::source {
    enum class NumberTupleField : std::uint8_t {
        NUMBER
    };

    using NumberTuple = core::tuple::TupleT<core::tuple::TupleFieldT<int, NumberTupleField::NUMBER>>;

    class ConstNumberSourceOperator final : public core::opr::TypedSourceOperator<NumberTuple> {
    public:
        void run() {
            emit(NumberTuple{1});
        }
    };
}

#endif //SIERRA_ODYPLAN_V1_TEST_OPERATOR_SOURCE_CONSTNUMBERSOURCEOPERATOR_H