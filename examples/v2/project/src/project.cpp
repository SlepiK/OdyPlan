#include <iostream>
#include <llvm/Support/TargetSelect.h>

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Support/raw_os_ostream.h>

#include "common/operator/sink/print_operator.h"
#include "core/logical/plan/plan.h"

#include "const_source_operator.h"
#include "core/builder/plan_builder.h"

#include <atomic>

#include "common/operator/pipe/project_operator.h"
#include "core/runtime/helper/bridge.h"

namespace {
    inline std::atomic atStop{false};
}

extern "C" {
    void rtEnd() {
    }

    void rtOdyPrintString(const char* data) {
        std::cout << data;
    }

    void rtOdyPrintDouble(const double value) {
        std::cout << value;
    }

    void rtOdyPrintFloat(const float value) {
        std::cout << value;
    }

    void rtOdyPrintInt(const int value) {
        std::cout << value;
    }

    void rtOdyPrintSeperator() {
        std::cout << " ";
    }

    void rtOdyPrintNewline() {
        std::cout << "\n";
    }

    bool odyStop() {
        return atStop.load(std::memory_order_relaxed);
    }
}

int main() {
    sierra::odyplan::v2::core::logical::plan::Plan logicalPlan;

    std::cout << "Building Logical Plan...\n";

    auto sourceId = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v2::example::project::logical::ConstantSourceOperator>());

    auto pipeId = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v2::common::opr::pipe::logical::ProjectOperator>(
        sierra::odyplan::v2::common::opr::pipe::logical::ProjectOperator::IndexMapping {1}
    ));

    auto sinkId = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v2::common::opr::sink::logical::PrintOperator>(
        sierra::odyplan::v2::common::opr::sink::logical::PrintOperator::PortFieldIndexV {0}
    ));

    logicalPlan.connect(
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=sourceId,.portId=0},
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=pipeId,.portId=0});

    logicalPlan.connect(
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=pipeId,.portId=0},
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=sinkId,.portId=0});

    std::cout << "Logical Plan: OK\n";

    std::cout << "Building Physical Plan...\n";
    auto typeRegistry = sierra::odyplan::v2::core::common::type::TypeRegistry::withDefaults();

    sierra::odyplan::v2::core::builder::PlanBuilder builder(typeRegistry);

    builder.registerOperatorTransformation(
        sierra::odyplan::v2::example::project::logical::ConstantSourceOperator::TYPE_ID,
        [](const sierra::odyplan::v2::core::logical::opr::IOperator& op,
            const sierra::odyplan::v2::core::physical::port::Ports&,
            const sierra::odyplan::v2::core::physical::port::Ports& outputPorts) -> sierra::odyplan::v2::core::physical::transformation::OperatorTransformation {
            const auto& concrete = static_cast<const sierra::odyplan::v2::example::project::logical::ConstantSourceOperator&>(op);

            sierra::odyplan::v2::core::physical::transformation::OperatorTransformation ot;
            ot.oprs.push_back(std::make_unique<sierra::odyplan::v2::example::project::physical::ConstantSourceOperator>(outputPorts.front()));
            return ot;
        }
    );

    builder.registerOperatorTransformation(
        sierra::odyplan::v2::common::opr::sink::logical::PrintOperator::TYPE_ID,
        [](const sierra::odyplan::v2::core::logical::opr::IOperator& op,
            const sierra::odyplan::v2::core::physical::port::Ports& inputPorts,
            const sierra::odyplan::v2::core::physical::port::Ports&) -> sierra::odyplan::v2::core::physical::transformation::OperatorTransformation {
            const auto& concrete = static_cast<const sierra::odyplan::v2::common::opr::sink::logical::PrintOperator&>(op);

            sierra::odyplan::v2::core::physical::transformation::OperatorTransformation ot;
            ot.oprs.push_back(std::make_unique<sierra::odyplan::v2::common::opr::sink::physical::PrintOperator>(inputPorts.front(),concrete.getPortFieldIndices()));
            return ot;
        }
    );


    builder.registerOperatorTransformation(
        sierra::odyplan::v2::common::opr::pipe::logical::ProjectOperator::TYPE_ID,
        [](const sierra::odyplan::v2::core::logical::opr::IOperator& op,
            const sierra::odyplan::v2::core::physical::port::Ports& inputPorts,
            const sierra::odyplan::v2::core::physical::port::Ports& outputPorts) -> sierra::odyplan::v2::core::physical::transformation::OperatorTransformation {
            const auto& concrete = static_cast<const sierra::odyplan::v2::common::opr::pipe::logical::ProjectOperator&>(op);

            sierra::odyplan::v2::core::physical::transformation::OperatorTransformation ot;
            ot.oprs.push_back(std::make_unique<sierra::odyplan::v2::common::opr::pipe::physical::ProjectOperator>(inputPorts.front(), outputPorts.front(), concrete.getMapping()));
            return ot;
        }
    );

    auto ctx = std::make_unique<llvm::LLVMContext>();
    sierra::odyplan::v2::core::physical::plan::Plan const physicalPlan = builder.build(logicalPlan, *ctx);

    std::cout << "Physical Plan: OK\n";
    std::cout << "Building LLVM-IR...\n";

    auto module = std::make_unique<llvm::Module>("exampleProject",*ctx);
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
    symbolMap[jit->mangleAndIntern("rtTaskSpawn")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtTaskSpawn),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtTaskJoin")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtTaskJoin),llvm::JITSymbolFlags::Exported);

    symbolMap[jit->mangleAndIntern("rtOdyPrintString")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtOdyPrintString),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtOdyPrintDouble")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtOdyPrintDouble),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtOdyPrintFloat")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtOdyPrintFloat),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtOdyPrintInt")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtOdyPrintInt),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtOdyPrintSeperator")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtOdyPrintSeperator),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtOdyPrintNewline")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtOdyPrintNewline),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("odyStop")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&odyStop),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtEnd")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtEnd),llvm::JITSymbolFlags::Exported);

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
#ifdef ENABLE_LOOPED_SOURCE
    std::thread mainThread([&]() {
#endif
        odyMain();
#ifdef ENABLE_LOOPED_SOURCE
    });

    std::cout << "Running (press Enter to stop:\n";

    std::cin.get();
    atStop.store(true);
    mainThread.join();
#endif

    return 0;
}
