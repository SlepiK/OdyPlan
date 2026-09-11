#include <iostream>

#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/OptimizationLevel.h>

#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/CodeGen.h>

#include <core/logical/plan/plan.h>
#include <core/builder/plan_builder.h>

#include <basic/operators/source/read/logical/read_operator.h>
#include <basic/operators/source/read/physical/read_operator.h>

#include <basic/operators/sink/write/logical/write_operator.h>
#include <basic/operators/sink/write/physical/write_operator.h>

#include <basic/expression/physical/builtin_expressions.h>

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

    void initializeSupportedTargets() {
#ifdef ODYPLAN_TARGET_X86
        LLVMInitializeX86TargetInfo();
        LLVMInitializeX86Target();
        LLVMInitializeX86TargetMC();
        LLVMInitializeX86AsmPrinter();
        LLVMInitializeX86AsmParser();
#endif

#ifdef ODYPLAN_TARGET_AArch64
        LLVMInitializeAArch64TargetInfo();
        LLVMInitializeAArch64Target();
        LLVMInitializeAArch64TargetMC();
        LLVMInitializeAArch64AsmPrinter();
        LLVMInitializeAArch64AsmParser();
#endif
    }
}

int main(int argc, char** argv) {
    const std::string outputPath = argc > 1 ? argv[1] : "write_json_plan.o";
    const std::string targetTripleStr = argc > 2 ? argv[2] : llvm::sys::getDefaultTargetTriple();

    sierra::odyplan::v3::core::logical::plan::Plan logicalPlan;

    std::cout << "Building Logical Plan...\n";

    auto sourceId = logicalPlan.addOperator(sierra::odyplan::v3::basic::operators::source::read::logical::ReadOperator::fromAutoSchema(
        std::to_underlying(sierra::odyplan::v3::basic::io::BuiltinIOFormat::JSON),
        nlohmann::json{{"path", "read.json"}},
        sierra::odyplan::v3::basic::operators::source::read::details::LReadOperatorOptions{
            //.startTimestampPosition = "timestamp"
        }
    ));

    auto sinkId = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v3::basic::operators::sink::write::logical::WriteOperator>(std::to_underlying(sierra::odyplan::v3::basic::io::BuiltinIOFormat::JSON), nlohmann::json{{"path", "write.json"}}));

    logicalPlan.connect(
        sierra::odyplan::v3::core::shared::operators::OperatorPortReference {.operatorId=sourceId,.portId=0},
        sierra::odyplan::v3::core::shared::operators::OperatorPortReference {.operatorId=sinkId,.portId=0});

    std::cout << "Logical Plan: OK\n";

    std::cout << "Building Physical Plan...\n";
    auto codegenTypeRegistry = sierra::odyplan::v3::core::physical::types::CodegenTypeRegistry::withBuiltins();

    sierra::odyplan::v3::core::builder::PlanBuilder builder(codegenTypeRegistry);

    builder.registerOperatorTransformation(
        sierra::odyplan::v3::basic::operators::source::read::logical::ReadOperator::TYPE_ID,
        [](const sierra::odyplan::v3::core::logical::operators::IOperator& op,
            const sierra::odyplan::v3::core::physical::port::Ports&,
            const sierra::odyplan::v3::core::physical::port::Ports& outputPorts) -> sierra::odyplan::v3::core::builder::transformation::OperatorTransformation {
            // NOLINTNEXTLINE
            const auto& concrete = static_cast<const sierra::odyplan::v3::basic::operators::source::read::logical::ReadOperator&>(op);

            sierra::odyplan::v3::core::builder::transformation::OperatorTransformation ot;
            ot.operators.push_back(std::make_unique<sierra::odyplan::v3::basic::operators::source::read::physical::ReadOperator>(
            outputPorts.front(),
            concrete.getIOConfig(),
            concrete.getIOFormat(),
                concrete.getOutputPorts().at(0),
                concrete.getOptions()
                ));
            return ot;
        }
    );

    builder.registerOperatorTransformation(
        sierra::odyplan::v3::basic::operators::sink::write::logical::WriteOperator::TYPE_ID,
        [](const sierra::odyplan::v3::core::logical::operators::IOperator& op,
            const sierra::odyplan::v3::core::physical::port::Ports& inputPorts,
            const sierra::odyplan::v3::core::physical::port::Ports&) -> sierra::odyplan::v3::core::builder::transformation::OperatorTransformation {
            const auto& concrete = static_cast<const sierra::odyplan::v3::basic::operators::sink::write::logical::WriteOperator&>(op);

            sierra::odyplan::v3::core::builder::transformation::OperatorTransformation ot;
            ot.operators.push_back(std::make_unique<sierra::odyplan::v3::basic::operators::sink::write::physical::WriteOperator>(
            inputPorts.front(),
            concrete.getIOConfig(),
            concrete.getIOFormat(),
                concrete.getInputPorts().at(0)
                ));
            return ot;
        }
    );

    auto ctx = std::make_unique<llvm::LLVMContext>();
    sierra::odyplan::v3::core::physical::plan::Plan const physicalPlan = builder.build(logicalPlan, *ctx);

    std::cout << "Physical Plan: OK\n";

    std::cout << "Building LLVM-IR...\n";

    auto module = std::make_unique<llvm::Module>("example_write_json", *ctx);
    physicalPlan.codegen(*module, *ctx);

    llvm::raw_os_ostream ros(std::cout);
    std::cout << "\n\n--- Raw IR ---\n";
    module->print(ros, nullptr);
    ros.flush();


    std::cout << "Running LLVM-AoT Stuff...\n";

    initializeSupportedTargets();

    const llvm::Triple targetTriple(targetTripleStr);
    module->setTargetTriple(targetTriple);

    std::string lookupError;
    const auto* target = llvm::TargetRegistry::lookupTarget(targetTriple, lookupError);
    if (target == nullptr) {
        std::cerr << "Failed to lookup target: " << lookupError << "\n";
        return 1;
    }

    llvm::TargetOptions targetOptions;
    std::unique_ptr<llvm::TargetMachine> targetMachine(
        target->createTargetMachine(targetTriple, "generic", "", targetOptions, llvm::Reloc::PIC_));

    if (!targetMachine) {
        std::cerr << "Failed to create target machine\n";
        return 1;
    }

    module->setDataLayout(targetMachine->createDataLayout());
    optimizeLlvmModule(*module, llvm::OptimizationLevel::O2);

    std::cout << "\n\n--- Optimized IR ---\n";
    module->print(ros, nullptr);
    ros.flush();
    std::cout << "\n\n";

    std::error_code ec;
    llvm::raw_fd_ostream dest(outputPath, ec, llvm::sys::fs::OF_None);
    if (ec) {
        std::cerr << "Could not open output file: " << ec.message() << "\n";
        return 1;
    }

    llvm::legacy::PassManager pass;
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
        std::cerr << "TargetMachine cannot emit object file for this target\n";
        return 1;
    }
    pass.run(*module);
    dest.flush();

    std::cout << "Object file written to: " << outputPath << "\n";
    return 0;
}