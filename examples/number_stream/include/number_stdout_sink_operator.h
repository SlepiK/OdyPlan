#ifndef SIERRA_ODYPLAN_V1_EXAMPLE_NUMBERSTREAM_NUMBERSTDOUTSINKOPERATOR_H
#define SIERRA_ODYPLAN_V1_EXAMPLE_NUMBERSTREAM_NUMBERSTDOUTSINKOPERATOR_H

#include "core/operator/sinkoperator/typed_sink_operator.h"
#include "const_number_source_operator.h"
#include <iostream>

namespace sierra::odyplan::v1::example::number_stream {
    class NumberStdoutSinkOperator final : public core::opr::TypedSinkOperator<OperatorPort> {
    public:
        void ingest(std::integral_constant<std::size_t, 0>, const OperatorPort& t) override {
            std::cout << "Got Data\n";
        }
    };
}

#endif //SIERRA_ODYPLAN_V1_EXAMPLE_NUMBERSTREAM_NUMBERSTDOUTSINKOPERATOR_H