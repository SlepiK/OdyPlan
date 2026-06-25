#include "const_number_source_operator.h"
#include "number_stdout_sink_operator.h"

#include "core/plan/plan.h"

int main() {
    sierra::odyplan::v1::core::plan::Plan plan;
    const auto source = plan.add<sierra::odyplan::v1::example::number_stream::ConstNumberSourceOperator>();
    const auto sink = plan.add<sierra::odyplan::v1::example::number_stream::NumberStdoutSinkOperator>();

    plan.connect(source, sink);

    auto tst = "test";

    return 0;
}
