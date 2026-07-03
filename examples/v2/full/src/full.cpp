#include <iostream>
#include <llvm/Support/TargetSelect.h>

#include "core/logical/plan/plan.h"

#include "const_int_source_operator.h"
#include "print_int_sink_operator.h"
#include "core/builder/plan_builder.h"

extern "C" {
    void odyExamplePrintUint32(const std::uint32_t value) {
        std::cout << value << "\n";
    }
}

int main() {
    sierra::odyplan::v2::core::logical::plan::Plan logicalPlan;

    std::cout << "Building Logical Plan...\n";

    auto sourceId = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v2::example::full::logical::ConstantIntSourceOperator>());

    auto sinkId = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v2::example::full::logical::PrintIntSinkOperator>());

    logicalPlan.connect(
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=sourceId,.portId=0},
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=sinkId,.portId=0});

    std::cout << "Logical Plan: OK\n";

    std::cout << "Building Physical Plan...\n";
    sierra::odyplan::v2::core::physical::port::PortTypeRegistry typeRegistry;
    typeRegistry.registerType(sierra::odyplan::v2::example::full::TYPE_INT, [](llvm::LLVMContext& ctx) {return llvm::Type::getInt32Ty(ctx);});

    sierra::odyplan::v2::core::builder::PlanBuilder builder(typeRegistry);

    builder.registerOperatorTransformation(
        sierra::odyplan::v2::example::full::logical::ConstantIntSourceOperator::TYPE_ID,
        [](const sierra::odyplan::v2::core::logical::opr::IOperator& op,
            const sierra::odyplan::v2::core::physical::port::Ports&,
            const sierra::odyplan::v2::core::physical::port::Ports& outputPorts) -> sierra::odyplan::v2::core::physical::transformation::OperatorTransformation {
            const auto& concrete = static_cast<const sierra::odyplan::v2::example::full::logical::ConstantIntSourceOperator&>(op);

            sierra::odyplan::v2::core::physical::transformation::OperatorTransformation ot;
            ot.oprs.push_back(std::make_unique<sierra::odyplan::v2::example::full::physical::ConstantIntSourceOperator>(outputPorts.front()));
            return ot;
        }
    );

    builder.registerOperatorTransformation(
        sierra::odyplan::v2::example::full::logical::PrintIntSinkOperator::TYPE_ID,
        [](const sierra::odyplan::v2::core::logical::opr::IOperator& op,
            const sierra::odyplan::v2::core::physical::port::Ports& inputPorts,
            const sierra::odyplan::v2::core::physical::port::Ports&) -> sierra::odyplan::v2::core::physical::transformation::OperatorTransformation {
            const auto& concrete = static_cast<const sierra::odyplan::v2::example::full::logical::PrintIntSinkOperator&>(op);

            sierra::odyplan::v2::core::physical::transformation::OperatorTransformation ot;
            ot.oprs.push_back(std::make_unique<sierra::odyplan::v2::example::full::physical::PrintIntSinkOperator>(inputPorts.front()));
            return ot;
        }
    );

    llvm::LLVMContext ctx;

    sierra::odyplan::v2::core::physical::plan::Plan physicalPlan = builder.build(logicalPlan, ctx);

    std::cout << "Physical Plan: OK\n";
    std::cout << "Building LLVM-IR...\n";

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    auto module = std::make_unique<llvm::Module>("exampleFull",ctx);
    physicalPlan.codegen(*module, ctx);

    module->print(llvm::errs(), nullptr);

    std::cout << "LLVM-IR: OK\n";


    return 0;
}
