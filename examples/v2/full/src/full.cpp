#include <iostream>
#include <llvm/Support/TargetSelect.h>

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Support/raw_os_ostream.h>

#include "core/logical/plan/plan.h"

#include "const_int_source_operator.h"
#include "print_int_sink_operator.h"
#include "core/builder/plan_builder.h"

#include <atomic>

inline std::atomic<bool> atStop{false};

extern "C" {
    void odyExamplePrintUint32(const std::uint32_t value) {
        std::cout << value << "\n";
    }

    bool odyStop() {
        return atStop.load(std::memory_order_relaxed);
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

    auto ctx = std::make_unique<llvm::LLVMContext>();
    sierra::odyplan::v2::core::physical::plan::Plan physicalPlan = builder.build(logicalPlan, *ctx);

    std::cout << "Physical Plan: OK\n";
    std::cout << "Building LLVM-IR...\n";

    auto module = std::make_unique<llvm::Module>("exampleFull",*ctx);
    physicalPlan.codegen(*module, *ctx);

    std::cout << "\n\n";
    llvm::raw_os_ostream ros(std::cout);
    module->print(ros, nullptr);
    ros.flush();
    std::cout << "\n\n";

    std::cout << "LLVM-IR: OK\n";
    std::cout << "Running via LLVM-JIT...\n";

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    auto jitExpected = llvm::orc::LLJITBuilder().create();
    if (!jitExpected) {
        std::cerr << "Failed to create JIT: " << llvm::toString(jitExpected.takeError()) << "\n";
        return 1;
    }

    auto& jit = *jitExpected;

    auto& dylib = jit->getMainJITDylib();
    llvm::orc::SymbolMap symbolMap;
    symbolMap[jit->mangleAndIntern("odyExamplePrintUint32")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&odyExamplePrintUint32),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("odyStop")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&odyStop),llvm::JITSymbolFlags::Exported);

    if (auto err = dylib.define(llvm::orc::absoluteSymbols(symbolMap))) {
        std::cerr << "Failed to register symbol: " << llvm::toString(std::move(err)) << "\n";
        return 1;
    }

    auto threadSafeModule = llvm::orc::ThreadSafeModule(std::move(module),std::move(ctx));

    if (auto err = jit->addIRModule(std::move(threadSafeModule))) {
        std::cerr << "Failed to add IR module: " << llvm::toString(std::move(err)) << "\n";
        return 1;
    }

    auto mainExpected = jit->lookup("ody_main");

    if (!mainExpected) {
        std::cerr << "Failed to find ody_main: " << llvm::toString(mainExpected.takeError()) << "\n";
        return 1;
    }

    auto odyMain = mainExpected->toPtr<void(*)()>();

    std::thread mainThread([&]() {
        odyMain();
    });

    std::cout << "Running (press Enter to stop:\n";

    std::cin.get();
    atStop.store(true);
    mainThread.join();

    return 0;
}
