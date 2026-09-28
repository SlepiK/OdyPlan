#include <basic/operators/pipe/join/logical/join_operator.h>
#include <basic/operators/pipe/join/runtime/join_runtime.h>

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Passes/OptimizationLevel.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_os_ostream.h>

#include <basic/operators/sink/print/logical/print_operator.h>
#include <basic/operators/sink/print/runtime/print_runtime.h>
#include <basic/operators/source/read/logical/read_operator.h>
#include <basic/operators/source/read/runtime/read_runtime.h>
#include <basic/operators/operator_transformations.h>
#include <core/builder/plan_builder.h>
#include <core/logical/plan/plan.h>
#include <core/runtime/core_runtime.h>
#include <core/runtime/time_runtime.h>
#include <core/runtime/type_runtime.h>
#include <core/runtime/memory/fan_in_memory.h>
#include <core/runtime/compute_segment_runtime.h>
#include <iostream>
#include <llvm/IR/Verifier.h>

#include "basic/expression/logical/compare_expression.h"
#include "basic/expression/logical/field_reference_expression.h"

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

        auto sourceLeftId = logicalPlan.addOperator(sierra::odyplan::v3::basic::operators::source::read::logical::ReadOperator::fromAutoSchema(
            std::to_underlying(sierra::odyplan::v3::basic::io::BuiltinIOFormat::NDJSON),
            nlohmann::json{{"path", "nexmark.person.10k.ndjson"}},
            sierra::odyplan::v3::basic::operators::source::read::details::LReadOperatorOptions{
                .startTimestampPosition = "timestamp"
            }
        ));

        auto sourceRightId = logicalPlan.addOperator(sierra::odyplan::v3::basic::operators::source::read::logical::ReadOperator::fromAutoSchema(
            std::to_underlying(sierra::odyplan::v3::basic::io::BuiltinIOFormat::NDJSON),
            nlohmann::json{{"path", "nexmark.bid.10k.ndjson"}},
            sierra::odyplan::v3::basic::operators::source::read::details::LReadOperatorOptions{
                .startTimestampPosition = "timestamp"
            }
        ));

        auto joinPredicate = std::make_unique<sierra::odyplan::v3::basic::expression::logical::CompareExpression>(
            std::make_unique<sierra::odyplan::v3::basic::expression::logical::FieldReferenceExpression>("id", 0),
            sierra::odyplan::v3::basic::expression::CompareOp::EQ,
            std::make_unique<sierra::odyplan::v3::basic::expression::logical::FieldReferenceExpression>("bidder", 1)
        );

        auto joinConfig = sierra::odyplan::v3::basic::operators::pipe::join::details::JoinOperatorOptions{
            .allowedLateness = std::chrono::milliseconds(1).count(),
            .card = sierra::odyplan::v3::basic::operators::pipe::join::details::JoinOperatorCard::ONE_MANY
        };

        auto joinId = logicalPlan.addOperator(
            std::make_unique<sierra::odyplan::v3::basic::operators::pipe::join::logical::JoinOperator>(
                joinConfig,
                std::move(joinPredicate),
                true
            ));

        auto sinkId = logicalPlan.addOperator(
            std::make_unique<sierra::odyplan::v3::basic::operators::sink::print::logical::PrintOperator>(
            sierra::odyplan::v3::basic::operators::sink::print::details::PrintOperatorOptions{
        //.printPunctuation = true,
    }
            ));

        logicalPlan.connect(
            sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                .operatorId = sourceLeftId, .portId = 0
            },
            sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                .operatorId = joinId, .portId = 0
            });

        logicalPlan.connect(
            sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                .operatorId = sourceRightId, .portId = 0
            },
            sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                .operatorId = joinId, .portId = 1
            });

        logicalPlan.connect(
            sierra::odyplan::v3::core::shared::operators::OperatorPortReference{
                .operatorId = joinId, .portId = 0
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

        auto module = std::make_unique<llvm::Module>("example_join", *ctx);
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
        symbolMap[jit->mangleAndIntern("rtJoinCreate")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinCreate),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinDestroy")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinDestroy),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinLock")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinLock),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinBeginLeft")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinBeginLeft),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinBeginRight")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinBeginRight),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinPushLeft")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinPushLeft),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinPushRight")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinPushRight),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinEndLeft")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinEndLeft),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinEndRight")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinEndRight),
            llvm::JITSymbolFlags::Exported);

        symbolMap[jit->mangleAndIntern("rtJoinMarkCandidateForRemove")] =
                llvm::orc::ExecutorSymbolDef(
                    llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::join::
                        runtime::rtJoinMarkCandidateForRemove),
                        llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinMarkCandidateAsMatch")] =
                llvm::orc::ExecutorSymbolDef(
                    llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::join::
                        runtime::rtJoinMarkCandidateAsMatch),
                        llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinRemoveMarkedCandidates")] =
                llvm::orc::ExecutorSymbolDef(
                    llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::join::
                        runtime::rtJoinRemoveMarkedCandidates),
                        llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinMarkAllCandidatesAsMatch")] =
                llvm::orc::ExecutorSymbolDef(
                    llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::join::
                        runtime::rtJoinMarkAllCandidatesAsMatch),
                        llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinSnapshotGetMatchCount")] =
                llvm::orc::ExecutorSymbolDef(
                    llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::join::
                        runtime::rtJoinSnapshotGetMatchCount),
                        llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinSnapshotGetMatchIndex")] =
                llvm::orc::ExecutorSymbolDef(
                    llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::join::
                        runtime::rtJoinSnapshotGetMatchIndex),
                    llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinSnapshotForCompute")] =
                llvm::orc::ExecutorSymbolDef(
                    llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::join::
                        runtime::rtJoinSnapshotForCompute),
                    llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinSnapshotGetOwnPtr")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinSnapshotGetOwnPtr),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinSnapshotGetCandidatePtr")] =
                llvm::orc::ExecutorSymbolDef(
                    llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::join::
                        runtime::rtJoinSnapshotGetCandidatePtr),
                    llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinSnapshotGetCount")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinSnapshotGetCount),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinSnapshotRelease")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinSnapshotRelease),
                llvm::JITSymbolFlags::Exported);


        symbolMap[jit->mangleAndIntern("rtJoinRemoveOutputRows")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinRemoveOutputRows),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinSnapshotGetOwnRowSize")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinSnapshotGetOwnRowSize),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinGetOutputRowEndTb")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinGetOutputRowEndTb),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinSnapshotGetOwnRowPtr")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinSnapshotGetOwnRowPtr),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinGetOutputRowStartTb")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinGetOutputRowStartTb),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinBufferOutput")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinBufferOutput),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinGetOutputRowEndTs")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinGetOutputRowEndTs),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinGetOutputRowStartTs")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinGetOutputRowStartTs),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinSnapshotGetCandidateRowSize")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinSnapshotGetCandidateRowSize),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinGetOutputRowPtr")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinGetOutputRowPtr),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinGetMinStartTimestampLeft")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinGetMinStartTimestampLeft),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinSnapshotGetCandidateRowPtr")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinSnapshotGetCandidateRowPtr),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinDrainOutput")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinDrainOutput),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinGetMinStartTimestampRight")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinGetMinStartTimestampRight),
                llvm::JITSymbolFlags::Exported);

        symbolMap[jit->mangleAndIntern("rtJoinSnapshotGetOwnEndTimestamp")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinSnapshotGetOwnEndTimestamp),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinSnapshotGetOwnStartTibreaker")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinSnapshotGetOwnStartTibreaker),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinSnapshotGetCandidateStartTimestamp")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinSnapshotGetCandidateStartTimestamp),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinSnapshotGetCandidateEndTibreaker")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinSnapshotGetCandidateEndTibreaker),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinSnapshotGetOwnEndTibreaker")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinSnapshotGetOwnEndTibreaker),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinSnapshotGetCandidateStartTibreaker")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinSnapshotGetCandidateStartTibreaker),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinSnapshotGetCandidateEndTimestamp")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinSnapshotGetCandidateEndTimestamp),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinSnapshotGetOwnStartTimestamp")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinSnapshotGetOwnStartTimestamp),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinMatchRight")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinMatchRight),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinMatchLeft")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinMatchLeft),
                llvm::JITSymbolFlags::Exported);

        symbolMap[jit->mangleAndIntern("rtJoinUnlock")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinUnlock),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinEvict")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinEvict),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinWatermarkCreate")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinWatermarkCreate),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinWatermarkDestroy")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinWatermarkDestroy),
                llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinUpdateWatermark")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinUpdateWatermark),
            llvm::JITSymbolFlags::Exported);

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
