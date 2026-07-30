#define SIERRA_RUNTIME_IO

#include <iostream>
#include <llvm/Support/TargetSelect.h>

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/OptimizationLevel.h>

#include "common/operator/source/read/logical/read_operator.h"
#include "common/operator/source/read/physical/read_operator.h"
#include "common/operator/source/read/runtime/read_runtime.h"

#include "common/operator/sink/write/logical/write_operator.h"
#include "common/operator/sink/write/physical/write_operator.h"
#include "common/operator/sink/write/runtime/write_runtime.h"

#include "common/runtime/runtime.h"

#include "core/logical/plan/plan.h"

#include "core/builder/plan_builder.h"

#include <atomic>

#include "common/io/io_registry.h"

void optimizeLlvmModule(llvm::Module& module, llvm::OptimizationLevel level) {
    //Passes are selected by reading some forum entries... , no real clue if they are the right ones... ;)
    llvm::LoopAnalysisManager loopAnaMgr;
    llvm::FunctionAnalysisManager fnAnaMgr;
    llvm::ModuleAnalysisManager modAnaMgr;
    llvm::CGSCCAnalysisManager cgsccAnaMgr;

    llvm::PassBuilder passBuilder;

    passBuilder.registerLoopAnalyses(loopAnaMgr);
    passBuilder.registerFunctionAnalyses(fnAnaMgr);
    passBuilder.registerModuleAnalyses(modAnaMgr);
    passBuilder.registerCGSCCAnalyses(cgsccAnaMgr);
    passBuilder.crossRegisterProxies(loopAnaMgr, fnAnaMgr, cgsccAnaMgr, modAnaMgr);

    llvm::ModulePassManager modPassMgr = passBuilder.buildPerModuleDefaultPipeline(level);

    modPassMgr.run(module, modAnaMgr);
}

int main() {
    sierra::odyplan::v2::core::logical::plan::Plan logicalPlan;

    std::cout << "Building Logical Plan...\n";

    auto sourceOutput = sierra::odyplan::v2::core::logical::port::Port {
        sierra::odyplan::v2::core::logical::port::PortDescriptor {.typeId= std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::INT64),.typeJsonName="int", .fieldName = "id"},
        sierra::odyplan::v2::core::logical::port::PortDescriptor {.typeId= std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::DOUBLE),.typeJsonName="double", .fieldName = "value"},
        sierra::odyplan::v2::core::logical::port::PortDescriptor {.typeId= std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::STRING),.typeJsonName="string", .fieldName = "name"}
    };

    auto sourceId = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v2::common::opr::source::logical::ReadOperator>(sourceOutput, sierra::odyplan::v2::common::io::IOFormat::JSON, nlohmann::json{{"path", "read.json"}}));

    auto sinkId = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v2::common::opr::sink::logical::WriteOperator>(sierra::odyplan::v2::common::io::IOFormat::JSON, nlohmann::json{{"path", "write.json"}}));

    logicalPlan.connect(
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=sourceId,.portId=0},
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=sinkId,.portId=0});

    std::cout << "Logical Plan: OK\n";

    std::cout << "Building Physical Plan...\n";
    auto typeRegistry = sierra::odyplan::v2::core::common::type::TypeRegistry::withDefaults();
    auto ioRegistry = sierra::odyplan::v2::common::io::IORegistry::withDefaults();

    sierra::odyplan::v2::core::builder::PlanBuilder builder(typeRegistry);

    builder.registerOperatorTransformation(
        sierra::odyplan::v2::common::opr::source::logical::ReadOperator::TYPE_ID,
        [&ioRegistry](const sierra::odyplan::v2::core::logical::opr::IOperator& op,
            const sierra::odyplan::v2::core::physical::port::Ports&,
            const sierra::odyplan::v2::core::physical::port::Ports& outputPorts) -> sierra::odyplan::v2::core::physical::transformation::OperatorTransformation {
            const auto& concrete = static_cast<const sierra::odyplan::v2::common::opr::source::logical::ReadOperator&>(op);

            sierra::odyplan::v2::core::physical::transformation::OperatorTransformation ot;
            ot.oprs.push_back(std::make_unique<sierra::odyplan::v2::common::opr::source::physical::ReadOperator>(
                outputPorts.front(),
                registerSource(ioRegistry.createSourceIO(concrete.getIOFormat(), concrete.getIOConfig())),
                concrete.getOutputPorts().at(0)
                ));
            return ot;
        }
    );

    builder.registerOperatorTransformation(
        sierra::odyplan::v2::common::opr::sink::logical::WriteOperator::TYPE_ID,
        [&ioRegistry](const sierra::odyplan::v2::core::logical::opr::IOperator& op,
            const sierra::odyplan::v2::core::physical::port::Ports& inputPorts,
            const sierra::odyplan::v2::core::physical::port::Ports&) -> sierra::odyplan::v2::core::physical::transformation::OperatorTransformation {
            const auto& concrete = static_cast<const sierra::odyplan::v2::common::opr::sink::logical::WriteOperator&>(op);

            sierra::odyplan::v2::core::physical::transformation::OperatorTransformation ot;
            ot.oprs.push_back(std::make_unique<sierra::odyplan::v2::common::opr::sink::physical::WriteOperator>(
                inputPorts.front(),
                registerSink(ioRegistry.createSinkIO(concrete.getIOFormat(), concrete.getIOConfig())),
                concrete.getInputPorts().at(0)
                ));
            return ot;
        }
    );

    auto ctx = std::make_unique<llvm::LLVMContext>();
    sierra::odyplan::v2::core::physical::plan::Plan const physicalPlan = builder.build(logicalPlan, *ctx);

    std::cout << "Physical Plan: OK\n";
    std::cout << "Building LLVM-IR...\n";

    auto module = std::make_unique<llvm::Module>("example_write_json",*ctx);
    physicalPlan.codegen(*module, *ctx);

    std::cout << "\n\n";
    std::cout << "\n\n--- Raw IR ---\n";
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

    module->setDataLayout(jit->getDataLayout());
    optimizeLlvmModule(*module, llvm::OptimizationLevel::O2);

    std::cout << "\n\n--- Optimized IR ---\n";
    module->print(ros, nullptr);
    ros.flush();
    std::cout << "\n\n";

    auto& dylib = jit->getMainJITDylib();
    llvm::orc::SymbolMap symbolMap;
    symbolMap[jit->mangleAndIntern("rtIOTaskSpawn")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtIOTaskSpawn),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtIOTaskJoin")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtIOTaskJoin),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtComputeTaskSpawn")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtComputeTaskSpawn),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtComputeTaskJoin")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtComputeTaskJoin),llvm::JITSymbolFlags::Exported);

    symbolMap[jit->mangleAndIntern("rtIOSinkBeginData")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtIOSinkBeginData),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtIOSinkWrite")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtIOSinkWrite),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtIOSinkEndData")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtIOSinkEndData),llvm::JITSymbolFlags::Exported);

    symbolMap[jit->mangleAndIntern("rtIOSourceHasNext")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtIOSourceHasNext),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtIOSourceGetField")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtIOSourceGetField),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtIOSourceReset")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtIOSourceReset),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtParseBool")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtParseBool),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtParseInt8")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtParseInt8),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtParseInt16")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtParseInt16),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtParseInt32")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtParseInt32),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtParseInt64")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtParseInt64),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtParseFloat")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtParseFloat),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtParseDouble")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtParseDouble),llvm::JITSymbolFlags::Exported);

    symbolMap[jit->mangleAndIntern("rtSleepMs")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtSleepMs), llvm::JITSymbolFlags::Exported);

    symbolMap[jit->mangleAndIntern("rtNow")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtNow),llvm::JITSymbolFlags::Exported);
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

    auto odyMain = jit->lookup("ody_main")->toPtr<void(*)()>();
    auto odyInit = jit->lookup("ody_init")->toPtr<void(*)(const void*)>();
    auto odyShutdown = jit->lookup("ody_shutdown")->toPtr<void(*)()>();

    odyInit(&typeRegistry);
    odyMain();
    odyShutdown();

    return 0;
}
