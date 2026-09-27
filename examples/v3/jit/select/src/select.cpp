#include <basic/operators/source/read/logical/read_operator.h>
#include <basic/operators/source/read/runtime/read_runtime.h>
#include "const_int_source_operator.h"

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Passes/OptimizationLevel.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_os_ostream.h>

#include <basic/operators/pipe/select/logical/select_operator.h>
#include <basic/operators/sink/print/logical/print_operator.h>
#include <basic/operators/sink/print/runtime/print_runtime.h>
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

#include "basic/expression/logical/builtin_expressions.h"

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

        auto sourceId = logicalPlan.addOperator(
            std::make_unique<
                sierra::odyplan::v3::example::select::logical::ConstantIntSourceOperator>());

        auto selectPredicate = sierra::odyplan::v3::basic::expression::logical::EQ(
            sierra::odyplan::v3::basic::expression::logical::field("testField1"),
            sierra::odyplan::v3::basic::expression::logical::cString("ProjectOperator_Test"));

        auto selectId = logicalPlan.addOperator(
            std::make_unique<
                sierra::odyplan::v3::basic::operators::pipe::select::logical::SelectOperator>(
                std::move(selectPredicate)));

        auto sinkId = logicalPlan.addOperator(
            std::make_unique<
                sierra::odyplan::v3::basic::operators::sink::print::logical::PrintOperator>(
                sierra::odyplan::v3::basic::operators::sink::print::details::PrintOperatorOptions{
                    .printPunctuation = true,
                }
                // sierra::odyplan::v3::basic::operators::sink::print::logical::PrintOperator::
                //PortFieldIndexV{0}
            ));

        logicalPlan.connect(
            sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                .operatorId = sourceId, .portId = 0
            },
            sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                .operatorId = selectId, .portId = 0
            });

        logicalPlan.connect(
            sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                .operatorId = selectId, .portId = 0
            },
            sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                .operatorId = sinkId, .portId = 0
            });

        std::cout << "Logical Plan: OK\n";

        std::cout << "Building Physical Plan...\n";
        auto codegebTypeRegistry =
                sierra::odyplan::v3::core::physical::types::CodegenTypeRegistry::withBuiltins();

        sierra::odyplan::v3::core::builder::PlanBuilder builder(codegebTypeRegistry);

        sierra::odyplan::v3::basic::operators::loadBuiltinOperatorTransformations(builder);

        builder.registerOperatorTransformation(
            sierra::odyplan::v3::example::select::logical::ConstantIntSourceOperator::TYPE_ID,
            [](sierra::odyplan::v3::core::logical::operators::IOperator &op,
               const sierra::odyplan::v3::core::physical::port::Ports &,
               const sierra::odyplan::v3::core::physical::port::Ports &outputPorts)
        -> sierra::odyplan::v3::core::builder::transformation::OperatorTransformation {
                // NOLINTBEGIN(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto &concrete = static_cast<const sierra::odyplan::v3::example::select::
                    logical::ConstantIntSourceOperator &>(op);
                // NOLINTEND(cppcoreguidelines-pro-type-static-cast-downcast)

                sierra::odyplan::v3::core::builder::transformation::OperatorTransformation ot;
                ot.operators.push_back(
                    std::make_unique<
                        sierra::odyplan::v3::example::select::physical::ConstantIntSourceOperator>(
                        outputPorts.front()));
                return ot;
            });

        auto ctx = std::make_unique<llvm::LLVMContext>();
        sierra::odyplan::v3::core::physical::plan::Plan const physicalPlan =
                builder.build(logicalPlan, *ctx);

        std::cout << "Physical Plan: OK\n";
        std::cout << "Building LLVM-IR...\n";

        auto module = std::make_unique<llvm::Module>("example_select", *ctx);
        physicalPlan.codegen(*module, *ctx);

        std::string errorInfo;
        llvm::raw_string_ostream errorStream(errorInfo);
        if (llvm::verifyModule(*module, &errorStream)) {
            throw std::runtime_error("Invalid LLVM IR generated: " + errorStream.str());
        }

        std::cout << "\n\n";
        llvm::raw_os_ostream ros(std::cout);
        std::cout << "\n\n--- Raw IR ---\n";
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

        symbolMap[jit->mangleAndIntern("rtPrintDataPackage")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::sink::print::runtime::rtPrintDataPackage),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtOdyPrintPunctuation")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::sink::print::runtime::rtOdyPrintPunctuation),
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
        auto odyInit = jit->lookup("ody_init")->toPtr<void (*)(const void *)>();
        auto odyShutdown = jit->lookup("ody_shutdown")->toPtr<void (*)()>();

        auto runtimeTypeRegistry =
                sierra::odyplan::v3::core::runtime::types::RuntimeTypeRegistry::withBuiltins();

        odyInit(&runtimeTypeRegistry);
        odyMain();
        odyShutdown();

        return 0;
    } catch (const std::exception &err) {
        std::cerr << err.what() << '\n';
        return 1;
    }
}
