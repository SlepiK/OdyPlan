#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Passes/OptimizationLevel.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_os_ostream.h>

#include <basic/operators/pipe/window/logical/count_window_operator.h>
#include <basic/operators/pipe/window/physical/count_window_operator.h>
#include <basic/operators/pipe/window/runtime/window_runtime.h>
#include <basic/operators/sink/print/logical/print_operator.h>
#include <basic/operators/sink/print/physical/print_operator.h>
#include <basic/operators/sink/print/runtime/print_runtime.h>
#include <basic/operators/source/read/logical/read_operator.h>
#include <basic/operators/source/read/physical/read_operator.h>
#include <basic/operators/source/read/runtime/read_runtime.h>
#include <core/builder/plan_builder.h>
#include <core/logical/plan/plan.h>
#include <core/runtime/core_runtime.h>
#include <core/runtime/time_runtime.h>
#include <iostream>

namespace {
    void optimizeLlvmModule(llvm::Module& module, const llvm::OptimizationLevel level) {
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
}  // namespace

// NOLINTNEXTLINE(readability-function-size)
int main() {
    try {
        sierra::odyplan::v3::core::logical::plan::Plan logicalPlan;

        std::cout << "Building Logical Plan...\n";

        auto sourceOutput = sierra::odyplan::v3::core::logical::port::Port{
            sierra::odyplan::v3::core::logical::port::PortDescription{
                .valueTypeId = std::to_underlying(
                    sierra::odyplan::v3::core::shared::types::BuiltinValueType::INT64),
                .typeJsonName = "int",
                .fieldName = "id"},
            sierra::odyplan::v3::core::logical::port::PortDescription{
                .valueTypeId = std::to_underlying(
                    sierra::odyplan::v3::core::shared::types::BuiltinValueType::DOUBLE),
                .typeJsonName = "double",
                .fieldName = "value"},
            sierra::odyplan::v3::core::logical::port::PortDescription{
                .valueTypeId = std::to_underlying(
                    sierra::odyplan::v3::core::shared::types::BuiltinValueType::STRING),
                .typeJsonName = "string",
                .fieldName = "name"}};

        auto sourceId = logicalPlan.addOperator(
            std::make_unique<
                sierra::odyplan::v3::basic::operators::source::read::logical::ReadOperator>(
                sourceOutput,
                std::to_underlying(sierra::odyplan::v3::basic::io::BuiltinIOFormat::NDJSON),
                nlohmann::json{{"path", "read.ndjson"}},
            std::chrono::milliseconds(5)));

        auto pipeId = logicalPlan.addOperator(
            std::make_unique<
                sierra::odyplan::v3::basic::operators::pipe::window::logical::CountWindowOperator>(
                sierra::odyplan::v3::basic::window::count::CountWindowConfig{
                    .type = sierra::odyplan::v3::basic::window::WindowType::SLIDING,
                    .size = 2,
                    .slide = 1}));

        auto sinkId = logicalPlan.addOperator(
            std::make_unique<
                sierra::odyplan::v3::basic::operators::sink::print::logical::PrintOperator>(
                sierra::odyplan::v3::basic::operators::sink::print::logical::PrintOperator::
                    PortFieldIndexV{0, 1, 2}));

        logicalPlan.connect(
            sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                .operatorId = sourceId, .portId = 0},
            sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                .operatorId = pipeId, .portId = 0});

        logicalPlan.connect(
            sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                .operatorId = pipeId, .portId = 0},
            sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                .operatorId = sinkId, .portId = 0});

        std::cout << "Logical Plan: OK\n";

        std::cout << "Building Physical Plan...\n";
        auto codegebTypeRegistry =
            sierra::odyplan::v3::core::physical::types::CodegenTypeRegistry::withBuiltins();

        sierra::odyplan::v3::core::builder::PlanBuilder builder(codegebTypeRegistry);

        builder.registerOperatorTransformation(
            sierra::odyplan::v3::basic::operators::source::read::logical::ReadOperator::TYPE_ID,
            [](const sierra::odyplan::v3::core::logical::operators::IOperator& op,
               const sierra::odyplan::v3::core::physical::port::Ports&,
               const sierra::odyplan::v3::core::physical::port::Ports& outputPorts)
                -> sierra::odyplan::v3::core::builder::transformation::OperatorTransformation {
                // NOLINTBEGIN(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto& concrete = static_cast<const sierra::odyplan::v3::basic::operators::
                                                       source::read::logical::ReadOperator&>(op);
                // NOLINTEND(cppcoreguidelines-pro-type-static-cast-downcast)

                sierra::odyplan::v3::core::builder::transformation::OperatorTransformation ot;
                ot.operators.push_back(
                    std::make_unique<sierra::odyplan::v3::basic::operators::source::read::physical::
                                         ReadOperator>(outputPorts.front(),
                                                       concrete.getOperatorTypeId(),
                                                       concrete.getIOConfig(),
                                                       concrete.getIOFormat(),
                                                       concrete.getOutputPorts().at(0),
                                             true,
                                             std::nullopt,
                                             concrete.getDelay()));
                return ot;
            });

        builder.registerOperatorTransformation(
            sierra::odyplan::v3::basic::operators::pipe::window::logical::CountWindowOperator::
                TYPE_ID,
            [](const sierra::odyplan::v3::core::logical::operators::IOperator& op,
               const sierra::odyplan::v3::core::physical::port::Ports& inputPorts,
               const sierra::odyplan::v3::core::physical::port::Ports& outputPorts)
                -> sierra::odyplan::v3::core::builder::transformation::OperatorTransformation {
                // NOLINTBEGIN(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto& concrete =
                    static_cast<const sierra::odyplan::v3::basic::operators::pipe::window::logical::
                                    CountWindowOperator&>(op);
                // NOLINTEND(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto& config = concrete.getWindowConfig();

                sierra::odyplan::v3::core::builder::transformation::OperatorTransformation ot;
                ot.operators.push_back(
                    std::make_unique<sierra::odyplan::v3::basic::operators::pipe::window::physical::
                                         CountWindowOperator>(inputPorts.front(),
                                                              outputPorts.front(),
                                                              config,
                                                              concrete.getInputPorts().front()));
                return ot;
            });

        builder.registerOperatorTransformation(
            sierra::odyplan::v3::basic::operators::sink::print::logical::PrintOperator::TYPE_ID,
            [](const sierra::odyplan::v3::core::logical::operators::IOperator& op,
               const sierra::odyplan::v3::core::physical::port::Ports& inputPorts,
               const sierra::odyplan::v3::core::physical::port::Ports&)
                -> sierra::odyplan::v3::core::builder::transformation::OperatorTransformation {
                // NOLINTBEGIN(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto& concrete = static_cast<const sierra::odyplan::v3::basic::operators::
                                                       sink::print::logical::PrintOperator&>(op);
                // NOLINTEND(cppcoreguidelines-pro-type-static-cast-downcast)

                sierra::odyplan::v3::core::builder::transformation::OperatorTransformation ot;
                ot.operators.push_back(std::make_unique<sierra::odyplan::v3::basic::operators::
                                                            sink::print::physical::PrintOperator>(
                    inputPorts.front(), concrete.getPortFieldIndices()));
                return ot;
            });

        auto ctx = std::make_unique<llvm::LLVMContext>();
        sierra::odyplan::v3::core::physical::plan::Plan const physicalPlan =
            builder.build(logicalPlan, *ctx);

        std::cout << "Physical Plan: OK\n";
        std::cout << "Building LLVM-IR...\n";

        auto module = std::make_unique<llvm::Module>("example_count_window", *ctx);
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
            std::cerr << "Failed to create JIT: " << llvm::toString(jitExpected.takeError())
                      << "\n";
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

        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        symbolMap[jit->mangleAndIntern("rtIOTaskSpawn")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::core::runtime::rtIOTaskSpawn),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtIOTaskJoin")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::core::runtime::rtIOTaskJoin),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtComputeTaskSpawn")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::core::runtime::rtComputeTaskSpawn),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtComputeTaskJoin")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::core::runtime::rtComputeTaskJoin),
            llvm::JITSymbolFlags::Exported);

        symbolMap[jit->mangleAndIntern("rtCreateCountWindowBuffer")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::window::
                                                 runtime::rtCreateCountWindowBuffer),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtDestroyCountWindowBuffer")] =
            llvm::orc::ExecutorSymbolDef(
                llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::
                                                     window::runtime::rtDestroyCountWindowBuffer),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtWindowGetStart")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::window::
                                                 runtime::rtWindowGetStart),
                                                 llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtWindowGetEnd")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::window::
                                                 runtime::rtWindowGetEnd),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtDestroyTimeWindowBuffer")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::window::
                                                 runtime::rtDestroyTimeWindowBuffer),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtWindowBeginRow")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::window::runtime::rtWindowBeginRow),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtWindowPush")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::window::runtime::rtWindowPush),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtWindowEndRow")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::window::runtime::rtWindowEndRow),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtWindowCompute")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::window::runtime::rtWindowCompute),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtWindowFlush")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::window::runtime::rtWindowFlush),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtWindowGetCount")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::window::runtime::rtWindowGetCount),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtWindowGetRowCountInWindow")] =
            llvm::orc::ExecutorSymbolDef(
                llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::
                                                     window::runtime::rtWindowGetRowCountInWindow),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtWindowGetData")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::window::runtime::rtWindowGetData),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtWindowGetTimestamp")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::window::
                                                 runtime::rtWindowGetTimestamp),
            llvm::JITSymbolFlags::Exported);

        symbolMap[jit->mangleAndIntern("rtIOSourceCreate")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::source::read::runtime::rtIOSourceCreate),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtIOSourceDestroy")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::source::read::runtime::rtIOSourceDestroy),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtIOSourceHasNext")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::source::read::runtime::rtIOSourceHasNext),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtIOSourceGetField")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::source::read::runtime::rtIOSourceGetField),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtIOSourceReset")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::source::read::runtime::rtIOSourceReset),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtParseBool")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::source::read::runtime::rtParseBool),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtParseInt8")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::source::read::runtime::rtParseInt8),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtParseInt16")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::source::read::runtime::rtParseInt16),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtParseInt32")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::source::read::runtime::rtParseInt32),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtParseInt64")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::source::read::runtime::rtParseInt64),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtParseFloat")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::source::read::runtime::rtParseFloat),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtParseDouble")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::source::read::runtime::rtParseDouble),
            llvm::JITSymbolFlags::Exported);

        symbolMap[jit->mangleAndIntern("rtOdyPrintString")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::sink::print::runtime::rtOdyPrintString),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtOdyPrintDouble")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::sink::print::runtime::rtOdyPrintDouble),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtOdyPrintFloat")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::sink::print::runtime::rtOdyPrintFloat),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtOdyPrintInt")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::sink::print::runtime::rtOdyPrintInt),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtOdyPrintSeperator")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::sink::print::runtime::rtOdyPrintSeperator),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtOdyPrintNewline")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::sink::print::runtime::rtOdyPrintNewline),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtOdyPrintWindowMetadata")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::sink::print::runtime::rtOdyPrintWindowMetadata),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtOdyPrintTimeStamp")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::sink::print::runtime::rtOdyPrintTimeStamp),
            llvm::JITSymbolFlags::Exported);

        symbolMap[jit->mangleAndIntern("rtSleepMs")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::core::runtime::rtSleepMs),
            llvm::JITSymbolFlags::Exported);

        symbolMap[jit->mangleAndIntern("rtTimeNow")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::core::runtime::rtTimeNow),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtProgramTimeNow")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::core::runtime::rtProgramTimeNow),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("odyStop")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::core::runtime::odyStop),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtEnd")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::core::runtime::rtEnd),
            llvm::JITSymbolFlags::Exported);
        // NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)

        if (auto err = dylib.define(llvm::orc::absoluteSymbols(symbolMap))) {
            std::cerr << "Failed to register symbol: " << llvm::toString(std::move(err)) << "\n";
            return 1;
        }

        auto threadSafeModule = llvm::orc::ThreadSafeModule(std::move(module), std::move(ctx));

        if (auto err = jit->addIRModule(std::move(threadSafeModule))) {
            std::cerr << "Failed to add IR module: " << llvm::toString(std::move(err)) << "\n";
            return 1;
        }

        auto odyMain = jit->lookup("ody_main")->toPtr<void (*)()>();
        auto odyInit = jit->lookup("ody_init")->toPtr<void (*)(const void*)>();
        auto odyShutdown = jit->lookup("ody_shutdown")->toPtr<void (*)()>();

        auto runtimeTypeRegistry =
            sierra::odyplan::v3::core::runtime::types::RuntimeTypeRegistry::withBuiltins();

        odyInit(&runtimeTypeRegistry);
        odyMain();
        odyShutdown();

        return 0;
    } catch (const std::exception& err) {
        std::cerr << err.what() << '\n';
        return 1;
    }
}
