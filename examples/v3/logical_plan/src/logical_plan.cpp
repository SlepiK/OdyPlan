#include "const_number_source_operator.h"
#include "int_pipe_operator.h"
#include "number_stdout_sink_operator.h"

#include <core/logical/plan/plan.h>
#include <iostream>

int main() {
    try {
        sierra::odyplan::v3::core::logical::plan::Plan plan;

        std::cout << "Running creation of successful pipeline]\n";

        const auto sourceOperatorId = plan.addOperator(
            std::make_unique<
                sierra::odyplan::v3::example::logical_plan::ConstNumberSourceOperator>());

        const auto pipeTmpOperatorId = plan.addOperator(
            std::make_unique<sierra::odyplan::v3::example::logical_plan::IntPipeOperator>());

        const auto sinkOperatorId = plan.addOperator(
            std::make_unique<
                sierra::odyplan::v3::example::logical_plan::NumberStdoutSinkOperator>());

        try {
            plan.connect(
                sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                    .operatorId = sourceOperatorId, .portId = 0},
                sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                    .operatorId = pipeTmpOperatorId, .portId = 0});

            plan.connect(
                sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                    .operatorId = pipeTmpOperatorId, .portId = 0},
                sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                    .operatorId = sinkOperatorId, .portId = 0});

            std::cout << "OK: logical plan built [SOURCE >-PORT_0-INT-PORT_0-> PIPE "
                         ">-PORT_0-INT-PORT_0-> SINK]\n";
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }

        std::cout << "Running creation of unsuccessful pipeline - Fan-in]\n";

        const auto sinkOperatorSecId = plan.addOperator(
            std::make_unique<
                sierra::odyplan::v3::example::logical_plan::DoubleNumberStdoutSinkOperator>());
        const auto sourceOperatorThiId = plan.addOperator(
            std::make_unique<
                sierra::odyplan::v3::example::logical_plan::ConstDoubleNumberSourceOperator>());

        try {
            plan.connect(
                sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                    .operatorId = sourceOperatorThiId, .portId = 0},
                sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                    .operatorId = sinkOperatorSecId, .portId = 0});
            plan.connect(
                sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                    .operatorId = sourceOperatorThiId, .portId = 0},
                sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                    .operatorId = sinkOperatorSecId, .portId = 0});
            std::cout << "Error: Fan-in on Port-Level not found]\n";
        } catch (const std::exception& e) {
            std::cout << "OK (Fan-in found): " << e.what() << "\n";
        }

        std::cout << "Running creation of unsuccessful pipeline - Cycle]\n";

        const auto pipeTmpOperatorSecId = plan.addOperator(
            std::make_unique<sierra::odyplan::v3::example::logical_plan::IntPipeOperator>());

        const auto pipeTmpOperatorThiId = plan.addOperator(
            std::make_unique<sierra::odyplan::v3::example::logical_plan::IntPipeOperator>());

        try {
            plan.connect(
                sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                    .operatorId = pipeTmpOperatorSecId, .portId = 0},
                sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                    .operatorId = pipeTmpOperatorThiId, .portId = 0});
            plan.connect(
                sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                    .operatorId = pipeTmpOperatorThiId, .portId = 0},
                sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                    .operatorId = pipeTmpOperatorSecId, .portId = 0});
            std::cout << "Error: Cycle while connecting not found]\n";
        } catch (const std::exception& e) {
            std::cout << "OK (Cycle found): " << e.what() << "\n";
        }

        return 0;
    } catch (const std::exception& err) {
        std::cerr << err.what() << '\n';
        return 1;
    }
}
