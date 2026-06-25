#ifndef SIERRA_ODYPLAN_V1_EXAMPLE_NUMBERSTREAM_CONSTNUMBERSOURCEOPERATOR_H
#define SIERRA_ODYPLAN_V1_EXAMPLE_NUMBERSTREAM_CONSTNUMBERSOURCEOPERATOR_H

#include "core/operator/sourceoperator/typed_source_operator.h"
#include "core/port/port.h"
#include <cstdint>

namespace sierra::odyplan::v1::example::number_stream {
    using OperatorPort = core::port::Port<core::port::PortField<int>>;

    class ConstNumberSourceOperator final : public core::opr::TypedSourceOperator<OperatorPort> {
    public:
        bool produce() override {
            //emit(NumberTuple{1});
            return true;
        }
    };
}

#endif //SIERRA_ODYPLAN_V1_EXAMPLE_NUMBERSTREAM_CONSTNUMBERSOURCEOPERATOR_H