#include <iostream>
#include <llvm/Support/TargetSelect.h>

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Support/raw_os_ostream.h>

#include "core/logical/plan/plan.h"

#include "const_int_source_operator.h"
#include "core/builder/plan_builder.h"

#include <atomic>

inline std::atomic<bool> atStop{false};

extern "C" {
    void rtOdyPrintString(const char* dataRaw, const std::uint64_t dataLength) {
        std::cout << std::string_view(dataRaw, dataLength);
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

    auto sourceId = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v2::example::project::logical::ConstantIntSourceOperator>());

    auto sinkId = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v2::common::opr::sink::logical::PrintOperator>(
        sierra::odyplan::v2::common::opr::sink::logical::PrintOperator::PortFieldIndexV {0}
    ));

    logicalPlan.connect(
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=sourceId,.portId=0},
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=sinkId,.portId=0});

    std::cout << "Logical Plan: OK\n";

    std::cout << "Building Physical Plan...\n";
    sierra::odyplan::v2::core::physical::port::PortTypeRegistry typeRegistry;
    typeRegistry.registerType(sierra::odyplan::v2::example::project::TYPE_INT, [](llvm::LLVMContext& ctx) {return llvm::Type::getInt32Ty(ctx);});
    typeRegistry.registerType(sierra::odyplan::v2::example::project::TYPE_STRING, [](llvm::LLVMContext& ctx) {
        return llvm::StructType::get(ctx, {
            //data_raw
            llvm::PointerType::get(ctx, 0),
            //data_length
            llvm::Type::getInt64Ty(ctx)
        });
    });

    sierra::odyplan::v2::core::builder::PlanBuilder builder(typeRegistry);

    builder.registerOperatorTransformation(
        sierra::odyplan::v2::example::project::logical::ConstantIntSourceOperator::TYPE_ID,
        [](const sierra::odyplan::v2::core::logical::opr::IOperator& op,
            const sierra::odyplan::v2::core::physical::port::Ports&,
            const sierra::odyplan::v2::core::physical::port::Ports& outputPorts) -> sierra::odyplan::v2::core::physical::transformation::OperatorTransformation {
            const auto& concrete = static_cast<const sierra::odyplan::v2::example::project::logical::ConstantIntSourceOperator&>(op);

            sierra::odyplan::v2::core::physical::transformation::OperatorTransformation ot;
            ot.oprs.push_back(std::make_unique<sierra::odyplan::v2::example::project::physical::ConstantIntSourceOperator>(outputPorts.front()));
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

    auto ctx = std::make_unique<llvm::LLVMContext>();
    sierra::odyplan::v2::core::physical::plan::Plan physicalPlan = builder.build(logicalPlan, *ctx);

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
    symbolMap[jit->mangleAndIntern("rtOdyPrintString")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtOdyPrintString),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtOdyPrintDouble")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtOdyPrintDouble),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtOdyPrintFloat")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtOdyPrintFloat),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtOdyPrintInt")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtOdyPrintInt),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtOdyPrintSeperator")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtOdyPrintSeperator),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtOdyPrintNewline")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtOdyPrintNewline),llvm::JITSymbolFlags::Exported);
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
