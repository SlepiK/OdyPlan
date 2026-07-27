#include <iostream>
#include <llvm/Support/TargetSelect.h>

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Support/raw_os_ostream.h>

#include "core/logical/plan/plan.h"

#include "core/builder/plan_builder.h"

#include <atomic>

#include "common/io/io_registry.h"

#include "common/operator/source/read/logical/read_operator.h"
#include "common/operator/source/read/physical/read_operator.h"
#include "common/operator/source/read/runtime/read_runtime.h"

#include "common/operator/pipe/merge/logical/merge_operator.h"
#include "common/operator/pipe/merge/physical/merge_operator.h"
#include "common/operator/pipe/merge/runtime/merge_runtime.h"

#include "common/operator/pipe/measure_operator/logical/measure_operator.h"
#include "common/operator/pipe/measure_operator/physical/measure_operator.h"
#include "common/operator/pipe/measure_operator/runtime/measure_runtime.h"

#include "common/operator/sink/print/logical/print_operator.h"
#include "common/operator/sink/print/physical/print_operator.h"
#include "common/operator/sink/print/runtime/print_runtime.h"

#include "core/runtime/helper/io_task.h"
#include "core/runtime/helper/compute_task.h"

namespace {
    inline std::atomic atStop{false};
}

extern "C" {
    void rtEnd() {
    }

    bool odyStop() {
        return atStop.load(std::memory_order_relaxed);
    }
}

int main() {
    sierra::odyplan::v2::core::logical::plan::Plan logicalPlan;

    std::cout << "Building Logical Plan...\n";

    auto sourceOutput = sierra::odyplan::v2::core::logical::port::Port {
        sierra::odyplan::v2::core::logical::port::PortDescriptor {.typeId= std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::INT64),.typeJsonName="int", .fieldName = "id"},
        sierra::odyplan::v2::core::logical::port::PortDescriptor {.typeId= std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::DOUBLE),.typeJsonName="double", .fieldName = "value"},
        sierra::odyplan::v2::core::logical::port::PortDescriptor {.typeId= std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::STRING),.typeJsonName="string", .fieldName = "name"}
    };

    auto sourceFirstId = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v2::common::opr::source::logical::ReadOperator>(sourceOutput, sierra::odyplan::v2::common::io::IOFormat::NDJSON, nlohmann::json{{"path", "read.ndjson"}}));

    auto sourceSecondId = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v2::common::opr::source::logical::ReadOperator>(sourceOutput, sierra::odyplan::v2::common::io::IOFormat::NDJSON, nlohmann::json{{"path", "read2.ndjson"}}));

    auto mergeId = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v2::common::opr::pipe::logical::MergeOperator>());

    auto measureId = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v2::common::opr::pipe::logical::MeasureOperator>("after_merge"));

    auto sinkId = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v2::common::opr::sink::logical::PrintOperator>(
        sierra::odyplan::v2::common::opr::sink::logical::PrintOperator::PortFieldIndexV {0,1,2}
    ));

    logicalPlan.connect(
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=sourceFirstId,.portId=0},
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=mergeId,.portId=0});

    logicalPlan.connect(
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=sourceSecondId,.portId=0},
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=mergeId,.portId=1});

    logicalPlan.connect(
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=mergeId,.portId=0},
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=measureId,.portId=0});

    logicalPlan.connect(
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=measureId,.portId=0},
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
                concrete.getOutputPorts().at(0)
                ));
            return ot;
        }
    );


    builder.registerOperatorTransformation(
        sierra::odyplan::v2::common::opr::pipe::logical::MeasureOperator::TYPE_ID,
        [&ioRegistry](const sierra::odyplan::v2::core::logical::opr::IOperator& op,
            const sierra::odyplan::v2::core::physical::port::Ports& inputPorts,
            const sierra::odyplan::v2::core::physical::port::Ports& outputPorts) -> sierra::odyplan::v2::core::physical::transformation::OperatorTransformation {
            // NOLINTNEXTLINE
            const auto& concrete = static_cast<const sierra::odyplan::v2::common::opr::pipe::logical::MeasureOperator&>(op);

            sierra::odyplan::v2::core::physical::transformation::OperatorTransformation ot;
            ot.oprs.push_back(std::make_unique<sierra::odyplan::v2::common::opr::pipe::physical::MeasureOperator>(
                    inputPorts.front(), outputPorts.front(), concrete.getLabel()
                ));
            return ot;
        }
    );

    builder.registerOperatorTransformation(
        sierra::odyplan::v2::common::opr::pipe::logical::MergeOperator::TYPE_ID,
        [](const sierra::odyplan::v2::core::logical::opr::IOperator& op,
            const sierra::odyplan::v2::core::physical::port::Ports& inputPorts,
            const sierra::odyplan::v2::core::physical::port::Ports& outputPorts) -> sierra::odyplan::v2::core::physical::transformation::OperatorTransformation {
            // NOLINTNEXTLINE
            const auto& concrete = static_cast<const sierra::odyplan::v2::common::opr::pipe::logical::MergeOperator&>(op);

            sierra::odyplan::v2::core::physical::transformation::OperatorTransformation ot;
            ot.oprs.push_back(std::make_unique<sierra::odyplan::v2::common::opr::pipe::physical::MergeOperator>(
                inputPorts.front(), outputPorts.front()
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

    auto module = std::make_unique<llvm::Module>("example_merge_measure",*ctx);
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

    symbolMap[jit->mangleAndIntern("rtCreateMeasure")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtCreateMeasure),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtDestroyMeasure")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtDestroyMeasure),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtMeasureTick")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtMeasureTick),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtMeasureFinalReport")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtMeasureFinalReport),llvm::JITSymbolFlags::Exported);

    symbolMap[jit->mangleAndIntern("rtCreateMergeLock")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtCreateMergeLock),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtDestroyMergeLock")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtDestroyMergeLock),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtMergeLock")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtMergeLock),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtMergeUnlock")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtMergeUnlock),llvm::JITSymbolFlags::Exported);

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

    std::fflush(nullptr);

    auto odyMain = jit->lookup("ody_main")->toPtr<void(*)()>();
    auto odyInit = jit->lookup("ody_init")->toPtr<void(*)(const void*)>();
    auto odyShutdown = jit->lookup("ody_shutdown")->toPtr<void(*)()>();

    odyInit(&typeRegistry);
    odyMain();
    odyShutdown();

    return 0;
}
