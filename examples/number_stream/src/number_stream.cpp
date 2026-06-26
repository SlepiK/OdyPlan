#include "const_number_source_operator.h"
#include "number_stdout_sink_operator.h"

#include "core/plan/plan.h"
#include "core/plan/plan_materializer.h"

int main() {
    sierra::odyplan::v1::core::plan::Plan plan;
    const auto source = plan.add<sierra::odyplan::v1::example::number_stream::ConstNumberSourceOperator>();
    const auto sink = plan.add<sierra::odyplan::v1::example::number_stream::NumberStdoutSinkOperator>();

    plan.connect(source, sink);

    auto converter = sierra::odyplan::v1::core::plan::PlanMaterializer();

    auto materializedPlan = converter.materialize(std::move(plan));

    for (auto sourceIdx : materializedPlan.getSourceOperators()) {
        auto& rawSource = materializedPlan.operatorAt(sourceIdx);
        auto& lowSource = static_cast<sierra::odyplan::v1::core::opr::ISourceOperator&>(rawSource);
        lowSource.produce();
        lowSource.produce();
        lowSource.produce();
        lowSource.produce();
    }

    return 0;
}
