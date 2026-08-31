#include "basic/operators/pipe/join/logical/join_operator.h"
#include "basic/operators/pipe/join/physical/join_operator.h"
#include "basic/operators/pipe/join/runtime/join_runtime.h"

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Passes/OptimizationLevel.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_os_ostream.h>

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

        auto sourceLeftOutput = sierra::odyplan::v3::core::logical::port::Port{
            sierra::odyplan::v3::core::logical::port::PortDescription{
                .valueTypeId = std::to_underlying(sierra::odyplan::v3::core::shared::types::BuiltinValueType::INT64),
                .typeJsonName = "int", .fieldName = "id"
            },
            sierra::odyplan::v3::core::logical::port::PortDescription{
                .valueTypeId = std::to_underlying(sierra::odyplan::v3::core::shared::types::BuiltinValueType::INT64),
                .typeJsonName = "int", .fieldName = "l_id"
            },
            sierra::odyplan::v3::core::logical::port::PortDescription{
                .valueTypeId = std::to_underlying(sierra::odyplan::v3::core::shared::types::BuiltinValueType::STRING),
                .typeJsonName = "string", .fieldName = "first_name"
            }
        };

        auto sourceLeftId = logicalPlan.addOperator(
            std::make_unique<sierra::odyplan::v3::basic::operators::source::read::logical::ReadOperator>(
                sourceLeftOutput,
                std::to_underlying(sierra::odyplan::v3::basic::io::BuiltinIOFormat::NDJSON),
                nlohmann::json{{"path", "left.ndjson"}},
                std::chrono::milliseconds(5)
            ));

        auto sourceRightOutput = sierra::odyplan::v3::core::logical::port::Port{
            sierra::odyplan::v3::core::logical::port::PortDescription{
                .valueTypeId = std::to_underlying(sierra::odyplan::v3::core::shared::types::BuiltinValueType::INT64),
                .typeJsonName = "int", .fieldName = "id"
            },
            sierra::odyplan::v3::core::logical::port::PortDescription{
                .valueTypeId = std::to_underlying(sierra::odyplan::v3::core::shared::types::BuiltinValueType::STRING),
                .typeJsonName = "string", .fieldName = "last_name"
            }
        };

        auto sourceRightId = logicalPlan.addOperator(
            std::make_unique<sierra::odyplan::v3::basic::operators::source::read::logical::ReadOperator>(
                sourceRightOutput,
                std::to_underlying(sierra::odyplan::v3::basic::io::BuiltinIOFormat::NDJSON),
                nlohmann::json{{"path", "right.ndjson"}},
                std::chrono::milliseconds(2)
            ));

        static auto joinPredicate = std::make_unique<sierra::odyplan::v3::basic::expression::CompareExpression>(
            std::make_unique<sierra::odyplan::v3::basic::expression::FieldReferenceExpression>(1),
            sierra::odyplan::v3::basic::expression::CompareExpression::CompareOp::EQ,
            std::make_unique<sierra::odyplan::v3::basic::expression::FieldReferenceExpression>(3)
        );

        auto joinConfig = sierra::odyplan::v3::basic::operators::pipe::join::logical::details::JoinConfig{
            .leftWindowSize = std::chrono::milliseconds(10),
            .rightWindowSize = std::chrono::milliseconds(10)
        };

        auto joinId = logicalPlan.addOperator(
            std::make_unique<sierra::odyplan::v3::basic::operators::pipe::join::logical::JoinOperator>(
                joinConfig,
                joinPredicate.get()
            ));

        auto sinkId = logicalPlan.addOperator(
            std::make_unique<sierra::odyplan::v3::basic::operators::sink::print::logical::PrintOperator>(
                sierra::odyplan::v3::basic::operators::sink::print::logical::PrintOperator::PortFieldIndexV{0, 2, 4}
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
        auto codegenTypeRegistry =
                sierra::odyplan::v3::core::physical::types::CodegenTypeRegistry::withBuiltins();

        sierra::odyplan::v3::core::builder::PlanBuilder builder(codegenTypeRegistry);

        builder.registerOperatorTransformation(
            sierra::odyplan::v3::basic::operators::source::read::logical::ReadOperator::TYPE_ID,
            [](const sierra::odyplan::v3::core::logical::operators::IOperator &op,
               const sierra::odyplan::v3::core::physical::port::Ports &,
               const sierra::odyplan::v3::core::physical::port::Ports &outputPorts)
        -> sierra::odyplan::v3::core::builder::transformation::OperatorTransformation {
                // NOLINTBEGIN(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto &concrete = static_cast<const sierra::odyplan::v3::basic::operators::
                    source::read::logical::ReadOperator &>(op);
                // NOLINTEND(cppcoreguidelines-pro-type-static-cast-downcast)

                sierra::odyplan::v3::core::builder::transformation::OperatorTransformation ot;
                ot.operators.push_back(
                    std::make_unique<sierra::odyplan::v3::basic::operators::source::read::physical::
                        ReadOperator>(outputPorts.front(),
                                      concrete.getIOConfig(),
                                      concrete.getIOFormat(),
                                      concrete.getOutputPorts().at(0),
                                      true,
                                      std::nullopt,
                                      concrete.getDelay()));
                return ot;
            });

        builder.registerOperatorTransformation(
            sierra::odyplan::v3::basic::operators::pipe::join::logical::JoinOperator::TYPE_ID,
            [](const sierra::odyplan::v3::core::logical::operators::IOperator& op,
                const sierra::odyplan::v3::core::physical::port::Ports& inputPorts,
                const sierra::odyplan::v3::core::physical::port::Ports& outputPorts) -> sierra::odyplan::v3::core::builder::transformation::OperatorTransformation {
                // NOLINTNEXTLINE
                const auto& concrete = static_cast<const sierra::odyplan::v3::basic::operators::pipe::join::logical::JoinOperator&>(op);

                const sierra::odyplan::v3::core::logical::port::Ports schemas{
                    concrete.getInputPorts().at(0), concrete.getInputPorts().at(1)
                };

                sierra::odyplan::v3::core::builder::transformation::OperatorTransformation ot;
                ot.operators.push_back(std::make_unique<sierra::odyplan::v3::basic::operators::pipe::join::physical::JoinOperator>(
                    inputPorts.front(), outputPorts.front(), concrete.getConfig(), schemas, concrete.getPredicate()
                ));
                return ot;
            }
        );

        builder.registerOperatorTransformation(
            sierra::odyplan::v3::basic::operators::sink::print::logical::PrintOperator::TYPE_ID,
            [](const sierra::odyplan::v3::core::logical::operators::IOperator &op,
               const sierra::odyplan::v3::core::physical::port::Ports &inputPorts,
               const sierra::odyplan::v3::core::physical::port::Ports &)
        -> sierra::odyplan::v3::core::builder::transformation::OperatorTransformation {
                // NOLINTBEGIN(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto &concrete = static_cast<const sierra::odyplan::v3::basic::operators::
                    sink::print::logical::PrintOperator &>(op);
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

        symbolMap[jit->mangleAndIntern("rtJoinSnapshotForComputeAndUnlock")] =
                llvm::orc::ExecutorSymbolDef(
                    llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::join::
                        runtime::rtJoinSnapshotForComputeAndUnlock),
                    llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinSnapshotGetOwnPtr")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinSnapshotGetOwnPtr),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinSnapshotGetOwnTimestamp")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::join::
                runtime::rtJoinSnapshotGetOwnTimestamp),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinSnapshotGetCandidatePtr")] =
                llvm::orc::ExecutorSymbolDef(
                    llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::join::
                        runtime::rtJoinSnapshotGetCandidatePtr),
                    llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinSnapshotGetCandidateTimestamp")] =
                llvm::orc::ExecutorSymbolDef(
                    llvm::orc::ExecutorAddr::fromPtr(&sierra::odyplan::v3::basic::operators::pipe::join::
                        runtime::rtJoinSnapshotGetCandidateTimestamp),
                    llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinSnapshotGetCount")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinSnapshotGetCount),
            llvm::JITSymbolFlags::Exported);
        symbolMap[jit->mangleAndIntern("rtJoinSnapshotRelease")] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(
                &sierra::odyplan::v3::basic::operators::pipe::join::runtime::rtJoinSnapshotRelease),
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
