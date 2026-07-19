#include <iostream>

#include "core/logical/plan/plan.h"

#include "const_number_source_operator.h"
#include "number_stdout_sink_operator.h"
#include "int_pipe_operator.h"

int main() {
    sierra::odyplan::v2::core::logical::plan::Plan plan;

    std::cout << "Running creation of successful pipeline]\n";

    const auto sourceOperatorId = plan.addOperator(std::make_unique<sierra::odyplan::v2::example::logical_plan::ConstNumberSourceOperator>());

    const auto pipeTmpOperatorId = plan.addOperator(std::make_unique<sierra::odyplan::v2::example::logical_plan::IntPipeOperator>());

    const auto sinkOperatorId = plan.addOperator(std::make_unique<sierra::odyplan::v2::example::logical_plan::NumberStdoutSinkOperator>());

    try {
        plan.connect(
            sierra::odyplan::v2::core::common::port::PortRef {.operatorId=sourceOperatorId,.portId=0},
            sierra::odyplan::v2::core::common::port::PortRef {.operatorId=pipeTmpOperatorId,.portId=0}
        );

        plan.connect(
            sierra::odyplan::v2::core::common::port::PortRef {.operatorId=pipeTmpOperatorId,.portId=0},
            sierra::odyplan::v2::core::common::port::PortRef {.operatorId=sinkOperatorId,.portId=0}
        );

        std::cout << "OK: logical plan built [SOURCE >-PORT_0-INT-PORT_0-> PIPE >-PORT_0-INT-PORT_0-> SINK]\n";
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
    }

    std::cout << "Running creation of unsuccessful pipeline - Fan-in]\n";

    const auto sinkOperatorSecId = plan.addOperator(std::make_unique<sierra::odyplan::v2::example::logical_plan::DoubleNumberStdoutSinkOperator>());
    const auto sourceOperatorThiId = plan.addOperator(std::make_unique<sierra::odyplan::v2::example::logical_plan::ConstDoubleNumberSourceOperator>());

    try {
        plan.connect(
        sierra::odyplan::v2::core::common::port::PortRef {.operatorId=sourceOperatorThiId,.portId=0},
        sierra::odyplan::v2::core::common::port::PortRef {.operatorId=sinkOperatorSecId,.portId=0}
        );
        plan.connect(
        sierra::odyplan::v2::core::common::port::PortRef {.operatorId=sourceOperatorThiId,.portId=0},
        sierra::odyplan::v2::core::common::port::PortRef {.operatorId=sinkOperatorSecId,.portId=0}
        );
        std::cout << "Error: Fan-in on Port-Level not found]\n";
    } catch (const std::exception& e) {
        std::cout << "OK (Fan-in found): " << e.what() << "\n";
    }

    std::cout << "Running creation of unsuccessful pipeline - Cycle]\n";

    const auto pipeTmpOperatorSecId = plan.addOperator(std::make_unique<sierra::odyplan::v2::example::logical_plan::IntPipeOperator>());

    const auto pipeTmpOperatorThiId = plan.addOperator(std::make_unique<sierra::odyplan::v2::example::logical_plan::IntPipeOperator>());

    try {
        plan.connect(
        sierra::odyplan::v2::core::common::port::PortRef {.operatorId=pipeTmpOperatorSecId,.portId=0},
        sierra::odyplan::v2::core::common::port::PortRef {.operatorId=pipeTmpOperatorThiId,.portId=0}
        );
        plan.connect(
        sierra::odyplan::v2::core::common::port::PortRef {.operatorId=pipeTmpOperatorThiId,.portId=0},
        sierra::odyplan::v2::core::common::port::PortRef {.operatorId=pipeTmpOperatorSecId,.portId=0}
        );
        std::cout << "Error: Cycle while connecting not found]\n";
    } catch (const std::exception& e) {
        std::cout << "OK (Cycle found): " << e.what() << "\n";
    }

    return 0;
}
