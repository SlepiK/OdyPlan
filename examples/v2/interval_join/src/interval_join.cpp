#include <iostream>
#include <llvm/Support/TargetSelect.h>

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Support/raw_os_ostream.h>

#include "common/operator/source/read/logical/read_operator.h"
#include "common/operator/source/read/physical/read_operator.h"
#include "common/operator/source/read/runtime/read_runtime.h"

#include "common/operator/pipe/interval_join/logical/interval_join_operator.h"
#include "common/operator/pipe/interval_join/physical/interval_join_operator.h"
#include "common/operator/pipe/interval_join/runtime/join_runtime.h"

#include "common/operator/sink/print/logical/print_operator.h"
#include "common/operator/sink/print/physical/print_operator.h"
#include "common/operator/sink/print/runtime/print_runtime.h"

#include "common/runtime/runtime.h"

#include "core/logical/plan/plan.h"

#include "core/builder/plan_builder.h"

#include <atomic>

#include "common/io/io_registry.h"

int main() {
    sierra::odyplan::v2::core::logical::plan::Plan logicalPlan;

    std::cout << "Building Logical Plan...\n";

    auto sourceLeftOutput = sierra::odyplan::v2::core::logical::port::Port {
        sierra::odyplan::v2::core::logical::port::PortDescriptor {.typeId= std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::INT64),.typeJsonName="int", .fieldName = "id"},
        sierra::odyplan::v2::core::logical::port::PortDescriptor {.typeId= std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::INT64),.typeJsonName="int", .fieldName = "l_id"},
        sierra::odyplan::v2::core::logical::port::PortDescriptor {.typeId= std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::STRING),.typeJsonName="string", .fieldName = "first_name"}
    };

    auto sourceLeftId = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v2::common::opr::source::logical::ReadOperator>(
        sourceLeftOutput,
        sierra::odyplan::v2::common::io::IOFormat::NDJSON, nlohmann::json{{"path", "left.ndjson"}},
        std::chrono::milliseconds(5)
        ));

    auto sourceRightOutput = sierra::odyplan::v2::core::logical::port::Port {
        sierra::odyplan::v2::core::logical::port::PortDescriptor {.typeId= std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::INT64),.typeJsonName="int", .fieldName = "id"},
        sierra::odyplan::v2::core::logical::port::PortDescriptor {.typeId= std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::STRING),.typeJsonName="string", .fieldName = "last_name"}
    };

    auto sourceRightId = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v2::common::opr::source::logical::ReadOperator>(
        sourceRightOutput,
        sierra::odyplan::v2::common::io::IOFormat::NDJSON, nlohmann::json{{"path", "right.ndjson"}},
        std::chrono::milliseconds(2)
        ));

    static auto joinPredicate = std::make_unique<sierra::odyplan::v2::core::common::expression::CompareExpression>(
        std::make_unique<sierra::odyplan::v2::core::common::expression::FieldReferenceExpression>(1),
        sierra::odyplan::v2::core::common::expression::CompareExpression::CompareOp::EQ,
        std::make_unique<sierra::odyplan::v2::core::common::expression::FieldReferenceExpression>(3)
    );

    auto joinConfig = sierra::odyplan::v2::common::opr::pipe::physical::internal::IntervalJoinConfig {
        .leftWindowSize = std::chrono::milliseconds(10),
        .rightWindowSize = std::chrono::milliseconds(10)
    };

    auto joinId = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v2::common::opr::pipe::logical::IntervalJoinOperator>(
        joinConfig,
        joinPredicate.get()
    ));

    auto sinkId = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v2::common::opr::sink::logical::PrintOperator>(
        sierra::odyplan::v2::common::opr::sink::logical::PrintOperator::PortFieldIndexV {0,2,4}
    ));

    logicalPlan.connect(
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=sourceLeftId,.portId=0},
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=joinId,.portId=0});

    logicalPlan.connect(
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=sourceRightId,.portId=0},
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=joinId,.portId=1});

    logicalPlan.connect(
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=joinId,.portId=0},
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
            // NOLINTNEXTLINE
            const auto& concrete = static_cast<const sierra::odyplan::v2::common::opr::source::logical::ReadOperator&>(op);

            sierra::odyplan::v2::core::physical::transformation::OperatorTransformation ot;
            ot.oprs.push_back(std::make_unique<sierra::odyplan::v2::common::opr::source::physical::ReadOperator>(
                outputPorts.front(),
                registerSource(ioRegistry.createSourceIO(concrete.getIOFormat(), concrete.getIOConfig())),
                concrete.getOutputPorts().at(0), std::nullopt, concrete.getDelay()
                ));
            return ot;
        }
    );

    builder.registerOperatorTransformation(
        sierra::odyplan::v2::common::opr::pipe::logical::IntervalJoinOperator::TYPE_ID,
        [](const sierra::odyplan::v2::core::logical::opr::IOperator& op,
            const sierra::odyplan::v2::core::physical::port::Ports& inputPorts,
            const sierra::odyplan::v2::core::physical::port::Ports& outputPorts) -> sierra::odyplan::v2::core::physical::transformation::OperatorTransformation {
            // NOLINTNEXTLINE
            const auto& concrete = static_cast<const sierra::odyplan::v2::common::opr::pipe::logical::IntervalJoinOperator&>(op);

            const sierra::odyplan::v2::core::logical::port::Ports schemas{
                concrete.getInputPorts().at(0), concrete.getInputPorts().at(1)
            };

            sierra::odyplan::v2::core::physical::transformation::OperatorTransformation ot;
            ot.oprs.push_back(std::make_unique<sierra::odyplan::v2::common::opr::pipe::physical::IntervalJoinOperator>(
                inputPorts.front(), outputPorts.front(), concrete.getConfig(), schemas, concrete.getPredicate()
            ));
            return ot;
        }
    );

    builder.registerOperatorTransformation(
        sierra::odyplan::v2::common::opr::sink::logical::PrintOperator::TYPE_ID,
        [](const sierra::odyplan::v2::core::logical::opr::IOperator& op,
            const sierra::odyplan::v2::core::physical::port::Ports& inputPorts,
            const sierra::odyplan::v2::core::physical::port::Ports&) -> sierra::odyplan::v2::core::physical::transformation::OperatorTransformation {
            // NOLINTNEXTLINE
            const auto& concrete = static_cast<const sierra::odyplan::v2::common::opr::sink::logical::PrintOperator&>(op);

            sierra::odyplan::v2::core::physical::transformation::OperatorTransformation ot;
            ot.oprs.push_back(std::make_unique<sierra::odyplan::v2::common::opr::sink::physical::PrintOperator>(inputPorts.front(),concrete.getPortFieldIndices()));
            return ot;
        }
    );

    auto ctx = std::make_unique<llvm::LLVMContext>();
    sierra::odyplan::v2::core::physical::plan::Plan const physicalPlan = builder.build(logicalPlan, *ctx);

    std::cout << "Physical Plan: OK\n";
    std::cout << "Building LLVM-IR...\n";

    auto module = std::make_unique<llvm::Module>("example_read_json",*ctx);
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

    symbolMap[jit->mangleAndIntern("rtJoinCreate")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtJoinCreate), llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtJoinDestroy")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtJoinDestroy), llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtJoinLock")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtJoinLock), llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtJoinUnlock")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtJoinUnlock), llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtJoinBeginLeft")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtJoinBeginLeft), llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtJoinBeginRight")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtJoinBeginRight), llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtJoinPushLeft")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtJoinPushLeft), llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtJoinPushRight")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtJoinPushRight), llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtJoinEndLeft")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtJoinEndLeft), llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtJoinEndRight")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtJoinEndRight), llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtJoinGetCandidateSize")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtJoinGetCandidateSize), llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtJoinGetCandidatePtr")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtJoinGetCandidatePtr), llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtJoinGetCandidateTimestamp")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtJoinGetCandidateTimestamp), llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtJoinEvict")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtJoinEvict), llvm::JITSymbolFlags::Exported);

    symbolMap[jit->mangleAndIntern("rtIOTaskSpawn")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtIOTaskSpawn),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtIOTaskJoin")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtIOTaskJoin),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtComputeTaskSpawn")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtComputeTaskSpawn),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtComputeTaskJoin")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtComputeTaskJoin),llvm::JITSymbolFlags::Exported);

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


    symbolMap[jit->mangleAndIntern("rtOdyPrintString")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtOdyPrintString),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtOdyPrintDouble")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtOdyPrintDouble),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtOdyPrintFloat")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtOdyPrintFloat),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtOdyPrintInt")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtOdyPrintInt),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtOdyPrintSeperator")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtOdyPrintSeperator),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtOdyPrintNewline")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtOdyPrintNewline),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtOdyPrintTimeStamp")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtOdyPrintTimeStamp),llvm::JITSymbolFlags::Exported);

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
