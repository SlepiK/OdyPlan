#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Passes/OptimizationLevel.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_os_ostream.h>

#include <basic/operators/pipe/union/logical/union_operator.h>
#include <basic/operators/pipe/union/runtime/union_runtime.h>
#include <basic/operators/sink/print/logical/print_operator.h>
#include <basic/operators/sink/print/runtime/print_runtime.h>
#include <basic/operators/source/read/logical/read_operator.h>
#include <basic/operators/source/read/runtime/read_runtime.h>
#include <basic/operators/operator_transformations.h>
#include <core/builder/plan_builder.h>
#include <core/logical/plan/plan.h>
#include <core/runtime/core_runtime.h>
#include <core/runtime/time_runtime.h>
#include <core/runtime/types/runtime_type_registry.h>
#include <core/runtime/type_runtime.h>
#include <core/runtime/memory/fan_in_memory.h>
#include <core/runtime/compute_segment_runtime.h>
#include <iostream>
#include <llvm/IR/Verifier.h>

namespace {
    void optimizeLlvmModule(llvm::Module &module, const llvm::OptimizationLevel level) {
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
} // namespace

// NOLINTNEXTLINE(readability-function-size)
int main() {
    try {
        sierra::odyplan::v3::core::logical::plan::Plan logicalPlan;

        std::cout << "Building Logical Plan...\n";

        auto sourceFirstId = logicalPlan.addOperator(sierra::odyplan::v3::basic::operators::source::read::logical::ReadOperator::fromAutoSchema(
            std::to_underlying(sierra::odyplan::v3::basic::io::BuiltinIOFormat::NDJSON),
            nlohmann::json{{"path", "read.ndjson"}},
            sierra::odyplan::v3::basic::operators::source::read::details::LReadOperatorOptions{
                .startTimestampPosition = "timestamp"
            }
        ));


        auto sourceSecondId = logicalPlan.addOperator(sierra::odyplan::v3::basic::operators::source::read::logical::ReadOperator::fromAutoSchema(
            std::to_underlying(sierra::odyplan::v3::basic::io::BuiltinIOFormat::NDJSON),
            nlohmann::json{{"path", "read.ndjson"}},
            sierra::odyplan::v3::basic::operators::source::read::details::LReadOperatorOptions{
                .startTimestampPosition = "timestamp"
            }
        ));

        auto unionId = logicalPlan.addOperator(
            std::make_unique<
                sierra::odyplan::v3::basic::operators::pipe::union_::logical::UnionOperator>(false));

        auto sinkId = logicalPlan.addOperator(
            std::make_unique<
                sierra::odyplan::v3::basic::operators::sink::print::logical::PrintOperator>(
                sierra::odyplan::v3::basic::operators::sink::print::details::PrintOperatorOptions{
        .printPunctuation = true,
    }));

        logicalPlan.connect(
            sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                .operatorId = sourceFirstId, .portId = 0
            },
            sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                .operatorId = unionId, .portId = 0
            });

        logicalPlan.connect(
            sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                .operatorId = sourceSecondId, .portId = 0
            },
            sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                .operatorId = unionId, .portId = 1
            });

        logicalPlan.connect(
            sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                .operatorId = unionId, .portId = 0
            },
            sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                .operatorId = sinkId, .portId = 0
            });

        std::cout << "Logical Plan: OK\n";

        std::cout << "Building Physical Plan...\n";

        sierra::odyplan::v3::core::builder::PlanBuilder builder{};

        sierra::odyplan::v3::basic::operators::loadBuiltinOperatorTransformations(builder);

        auto ctx = std::make_unique<llvm::LLVMContext>();
        sierra::odyplan::v3::core::physical::plan::Plan const physicalPlan =
                builder.build(logicalPlan, *ctx);

        std::cout << "Physical Plan: OK\n";
        std::cout << "Building LLVM-IR...\n";

        auto module = std::make_unique<llvm::Module>("example_union", *ctx);
        physicalPlan.codegen(*module, *ctx);

        std::cout << "\n\n";
        std::cout << "\n\n--- Raw IR ---\n";
        llvm::raw_os_ostream ros(std::cout);
        module->print(ros, nullptr);
        ros.flush();
        std::cout << "\n\n";

        std::string errorInfo;
        llvm::raw_string_ostream errorStream(errorInfo);
        if (llvm::verifyModule(*module, &errorStream)) {
            throw std::runtime_error("Invalid LLVM IR generated: " + errorStream.str());
        }

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

        auto &jit = *jitExpected;

        module->setDataLayout(jit->getDataLayout());
        optimizeLlvmModule(*module, llvm::OptimizationLevel::O2);

        std::cout << "\n\n--- Optimized IR ---\n";
        module->print(ros, nullptr);
        ros.flush();
        std::cout << "\n\n";

        auto &dylib = jit->getMainJITDylib();
        llvm::orc::SymbolMap symbolMap;

        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        symbolMap[jit->mangleAndIntern("rtIOTaskSpawn")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::core::runtime::rtIOTaskSpawn),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtIOTaskJoin")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::core::runtime::rtIOTaskJoin),
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
        symbolMap[jit->mangleAndIntern("rtParseString")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::source::read::runtime::rtParseString),
            llvm::JITSymbolFlags::Exported);

        symbolMap[jit->mangleAndIntern("rtComputeSegmentNullInit")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::core::runtime::rtComputeSegmentNullInit),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtComputeSegmentGetOrCreate")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::core::runtime::rtComputeSegmentGetOrCreate),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtComputeSegmentEnqueue")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::core::runtime::rtComputeSegmentEnqueue),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtComputeSegmentDestroyIfStarted")] =
                llvm::orc::ExecutorSymbolDef(
                    llvm::orc::ExecutorAddr::fromPtr(
                        &sierra::odyplan::v3::core::runtime::rtComputeSegmentDestroyIfStarted),
                        llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtComputeSegmentDrain")] =
                llvm::orc::ExecutorSymbolDef(
                    llvm::orc::ExecutorAddr::fromPtr(
                        &sierra::odyplan::v3::core::runtime::rtComputeSegmentDrain),
                    llvm::JITSymbolFlags::Exported);

        symbolMap[jit->mangleAndIntern("rtTypeRegistryRetain")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::core::runtime::rtTypeRegistryRetain),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtTypeRegistryRelease")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::core::runtime::rtTypeRegistryRelease),
            llvm::JITSymbolFlags::Exported);

        symbolMap[jit->mangleAndIntern("rtFanInAlloc")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::core::runtime::memory::rtFanInAlloc),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtFanInFree")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::core::runtime::memory::rtFanInFree),
            llvm::JITSymbolFlags::Exported);

        symbolMap[jit->mangleAndIntern("rtPrintDataPackage")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::sink::print::runtime::rtPrintDataPackage),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtOdyPrintPunctuation")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::sink::print::runtime::rtOdyPrintPunctuation),
            llvm::JITSymbolFlags::Exported);


        symbolMap[jit->mangleAndIntern("rtUnionUnlock")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::union_::runtime::rtUnionUnlock),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtUnionLock")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::union_::runtime::rtUnionLock),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtUnionBufferGetEndTimestamp")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::union_::runtime::rtUnionBufferGetEndTimestamp),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtUnionBufferGetFieldPtr")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::union_::runtime::rtUnionBufferGetFieldPtr),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtUnionBufferGetStartTibreaker")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::union_::runtime::rtUnionBufferGetStartTibreaker),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtUnionBufferGetMinStartTimestamp")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::union_::runtime::rtUnionBufferGetMinStartTimestamp),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtUnionBufferGetCount")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::union_::runtime::rtUnionBufferGetCount),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtUnionBufferEnd")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::union_::runtime::rtUnionBufferEnd),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtUnionGetGlobalWatermark")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::union_::runtime::rtUnionGetGlobalWatermark),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtUnionBufferGetStartTimestamp")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::union_::runtime::rtUnionBufferGetStartTimestamp),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtUnionBufferRemoveV")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::union_::runtime::rtUnionBufferRemoveV),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtUnionBufferPush")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::union_::runtime::rtUnionBufferPush),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtUnionBufferBegin")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::union_::runtime::rtUnionBufferBegin),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtUnionBufferGetEndTibreaker")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::union_::runtime::rtUnionBufferGetEndTibreaker),
            llvm::JITSymbolFlags::Exported);

        symbolMap[jit->mangleAndIntern("rtUnionBufferGetBatchElement")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::union_::runtime::rtUnionBufferGetBatchElement),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtUnionBufferGetReadyIndices")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::union_::runtime::rtUnionBufferGetReadyIndices),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtUnionBufferCreate")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::union_::runtime::rtUnionBufferCreate),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtUnionBufferDestroy")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::union_::runtime::rtUnionBufferDestroy),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtUnionUpdateWatermark")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::union_::runtime::rtUnionUpdateWatermark),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtUnionWatermarkDestroy")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::union_::runtime::rtUnionWatermarkDestroy),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtUnionWatermarkCreate")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::union_::runtime::rtUnionWatermarkCreate),
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
        auto odyInit = jit->lookup("ody_init")->toPtr<void (*)()>();
        auto odyShutdown = jit->lookup("ody_shutdown")->toPtr<void (*)()>();

        odyInit();
        odyMain();
        odyShutdown();

        return 0;
    } catch (const std::exception &err) {
        std::cerr << err.what() << '\n';
        return 1;
    }
}
