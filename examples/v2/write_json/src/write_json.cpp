#include <iostream>
#include <llvm/Support/TargetSelect.h>

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Support/raw_os_ostream.h>

#include "common/operator/sink/print_operator.h"
#include "core/logical/plan/plan.h"

#include "core/builder/plan_builder.h"

#include <atomic>

#include "common/io/io_registry.h"
#include "common/operator/pipe/select_operator.h"
#include "common/operator/source/read_operator.h"
#include "common/operator/sink/write_operator.h"

inline std::atomic<bool> atStop{false};

inline std::vector<std::unique_ptr<sierra::odyplan::v2::common::io::IIOSource>> gSources;
inline std::vector<std::unique_ptr<sierra::odyplan::v2::common::io::IIOSink>> gSinks;

inline sierra::odyplan::v2::common::io::IOSourceId registerSource(std::unique_ptr<sierra::odyplan::v2::common::io::IIOSource> source) {
    gSources.push_back(std::move(source));
    return gSources.size() - 1;
}

inline sierra::odyplan::v2::common::io::IOSinkId registerSink(std::unique_ptr<sierra::odyplan::v2::common::io::IIOSink> sink) {
    gSinks.push_back(std::move(sink));
    return gSinks.size() - 1;
}

extern "C" {
    bool rtIOSourceHasNext(const sierra::odyplan::v2::common::io::IOSourceId index) {
        assert(index < gSources.size() && "invalid IOSource");
        if (index >= gSources.size()) {
            return false;
        }

        return gSources.at(index)->available();
    }

    const char* rtIOSourceGetField(const sierra::odyplan::v2::common::io::IOSourceId index, const char* fieldName) {
        assert(index < gSources.size() && "invalid IOSource");
        if (index >= gSources.size() || fieldName == nullptr) {
            return nullptr;
        }

        const sierra::odyplan::v2::common::io::IOData& current = gSources.at(index)->current();
        const auto iter = current.find(fieldName);
        return iter != current.end() ? iter->second.c_str() : nullptr;
    }

    void rtIOSourceReset(const sierra::odyplan::v2::common::io::IOSourceId index) {
        assert(index < gSources.size() && "invalid IOSource");
        if (index < gSources.size()) {
            gSources.at(index)->reset();
        }
    }

    bool rtParseBool(const char* str) {
        if (str == nullptr) {
            throw std::invalid_argument("rtParseBool: invalid argument");
        }

        if (std::strcmp(str, "true") == 0 || std::strcmp(str, "1") == 0) {
            return true;
        }
        if (std::strcmp(str, "false") == 0 || std::strcmp(str, "0") == 0) {
            return false;
        }

        throw std::invalid_argument("rtParseBool: invalid argument");
    }

    std::int8_t rtParseInt8(const char* str) {
        if (str == nullptr) {
            throw std::invalid_argument("rtParseInt8: invalid argument");
        }

        char* end = nullptr;
        errno = 0;
        const long value = std::strtol(str, &end, 10);

        if (end == str || *end != '\0' || errno == ERANGE || value < std::numeric_limits<std::int8_t>::min() || value > std::numeric_limits<std::int8_t>::max()) {
            throw std::invalid_argument("rtParseInt8: invalid argument or out of range");
        }

        return static_cast<std::int8_t>(value);
    }

    std::int16_t rtParseInt16(const char* str) {
        if (str == nullptr) {
            throw std::invalid_argument("rtParseInt16: invalid argument");
        }

        char* end = nullptr;
        errno = 0;
        const long value = std::strtol(str, &end, 10);

        if (end == str || *end != '\0' || errno == ERANGE || value < std::numeric_limits<std::int16_t>::min() || value > std::numeric_limits<std::int16_t>::max()) {
            throw std::invalid_argument("rtParseInt16: invalid argument or out of range");
        }

        return static_cast<std::int16_t>(value);
    }

    std::int32_t rtParseInt32(const char* str) {
        if (str == nullptr) {
            throw std::invalid_argument("rtParseInt32: invalid argument");
        }

        char* end = nullptr;
        errno = 0;
        const long value = std::strtol(str, &end, 10);

        if (end == str || *end != '\0' || errno == ERANGE || value < std::numeric_limits<std::int32_t>::min() || value > std::numeric_limits<std::int32_t>::max()) {
            throw std::invalid_argument("rtParseInt32: invalid argument or out of range");
        }

        return static_cast<std::int32_t>(value);
    }

    std::int64_t rtParseInt64(const char* str) {
        if (str == nullptr) {
            throw std::invalid_argument("rtParseInt64: invalid argument");
        }

        char* end = nullptr;
        errno = 0;
        const long value = std::strtol(str, &end, 10);

        if (end == str || *end != '\0' || errno == ERANGE || value < std::numeric_limits<std::int64_t>::min() || value > std::numeric_limits<std::int64_t>::max()) {
            throw std::invalid_argument("rtParseInt64: invalid argument or out of range");
        }

        return static_cast<std::int64_t>(value);
    }

    std::float_t rtParseFloat(const char* str) {
        if (str == nullptr) {
            throw std::invalid_argument("rtParseFloat: invalid argument");
        }

        char* end = nullptr;
        errno = 0;
        const std::float_t value = std::strtof(str, &end);

        if (end == str || *end != '\0' || errno == ERANGE) {
            throw std::invalid_argument("rtParseFloat: invalid argument");
        }

        return value;
    }

    std::double_t rtParseDouble(const char* str) {
        if (str == nullptr) {
            throw std::invalid_argument("rtParseDouble: invalid argument");
        }

        char* end = nullptr;
        errno = 0;
        const std::double_t value = std::strtod(str, &end);

        if (end == str || *end != '\0' || errno == ERANGE) {
            throw std::invalid_argument("rtParseDouble: invalid argument");
        }

        return value;
    }

    void rtIOSinkBeginData(sierra::odyplan::v2::common::io::IOSinkId index) {

    }

    void rtIOSinkWriteString(sierra::odyplan::v2::common::io::IOSinkId index, const char* key, const char* value) {

    }

    void rtIOSinkWriteBool(sierra::odyplan::v2::common::io::IOSinkId index, const char* key, bool value) {

    }

    void rtIOSinkWriteInt8(sierra::odyplan::v2::common::io::IOSinkId index, const char* key, std::int8_t value) {

    }

    void rtIOSinkWriteInt16(sierra::odyplan::v2::common::io::IOSinkId index, const char* key, std::int16_t value) {

    }

    void rtIOSinkWriteInt32(sierra::odyplan::v2::common::io::IOSinkId index, const char* key, std::int32_t value) {

    }

    void rtIOSinkWriteInt64(sierra::odyplan::v2::common::io::IOSinkId index, const char* key, std::int64_t value) {

    }

    void rtIOSinkWriteFloat(sierra::odyplan::v2::common::io::IOSinkId index, const char* key, std::float_t value) {

    }

    void rtIOSinkWriteDouble(sierra::odyplan::v2::common::io::IOSinkId index, const char* key, std::double_t value) {

    }

    void rtIOSinkEndData(sierra::odyplan::v2::common::io::IOSinkId index) {

    }
    void rtOdyPrintString(const char* data) {
        std::cout << data;
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

    auto sourceOutput = sierra::odyplan::v2::core::logical::port::Port {
        sierra::odyplan::v2::core::logical::port::PortDescriptor {.typeId= std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::INT64),.typeJsonName="int", .fieldName = "id"},
        sierra::odyplan::v2::core::logical::port::PortDescriptor {.typeId= std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::DOUBLE),.typeJsonName="double", .fieldName = "value"},
        sierra::odyplan::v2::core::logical::port::PortDescriptor {.typeId= std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::STRING),.typeJsonName="string", .fieldName = "name"}
    };

    auto sourceId = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v2::common::opr::source::logical::ReadOperator>(sourceOutput, sierra::odyplan::v2::common::io::IOFormat::JSON, nlohmann::json{{"path", "read.json"}}));

    auto sinkId2 = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v2::common::opr::sink::logical::WriteOperator>(sierra::odyplan::v2::common::io::IOFormat::JSON, nlohmann::json{{"path", "write.json"}}));

    auto sinkId = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v2::common::opr::sink::logical::PrintOperator>(
        sierra::odyplan::v2::common::opr::sink::logical::PrintOperator::PortFieldIndexV {0,1,2}
    ));

    logicalPlan.connect(
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=sourceId,.portId=0},
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=sinkId,.portId=0});

    /*logicalPlan.connect(
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=selectId,.portId=0},
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=sinkId,.portId=0});*/

    std::cout << "Logical Plan: OK\n";

    std::cout << "Building Physical Plan...\n";
    auto typeRegistry = sierra::odyplan::v2::core::common::type::TypeRegistry::withDefaults();
    auto ioRegistry = sierra::odyplan::v2::common::io::IORegistry::withDefaults();

    sierra::odyplan::v2::core::builder::PlanBuilder builder(typeRegistry);

    builder.registerOperatorTransformation(
        sierra::odyplan::v2::common::opr::source::logical::ReadOperator::TYPE_ID,
        [&typeRegistry, &ioRegistry](const sierra::odyplan::v2::core::logical::opr::IOperator& op,
            const sierra::odyplan::v2::core::physical::port::Ports&,
            const sierra::odyplan::v2::core::physical::port::Ports& outputPorts) -> sierra::odyplan::v2::core::physical::transformation::OperatorTransformation {
            const auto& concrete = static_cast<const sierra::odyplan::v2::common::opr::source::logical::ReadOperator&>(op);

            sierra::odyplan::v2::core::physical::transformation::OperatorTransformation ot;
            ot.oprs.push_back(std::make_unique<sierra::odyplan::v2::common::opr::source::physical::ReadOperator>(
                outputPorts.front(),
                registerSource(ioRegistry.createSourceIO(concrete.getIOFormat(), concrete.getIOConfig())),
                typeRegistry,
                concrete.getOutputPorts().at(0)
                ));
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

    builder.registerOperatorTransformation(
        sierra::odyplan::v2::common::opr::pipe::logical::SelectOperator::TYPE_ID,
        [&typeRegistry](const sierra::odyplan::v2::core::logical::opr::IOperator& op,
            const sierra::odyplan::v2::core::physical::port::Ports& inputPorts,
            const sierra::odyplan::v2::core::physical::port::Ports& outputPorts) -> sierra::odyplan::v2::core::physical::transformation::OperatorTransformation {
            const auto& concrete = static_cast<const sierra::odyplan::v2::common::opr::pipe::logical::SelectOperator&>(op);

            sierra::odyplan::v2::core::physical::transformation::OperatorTransformation ot;
            ot.oprs.push_back(std::make_unique<sierra::odyplan::v2::common::opr::pipe::physical::SelectOperator>(inputPorts.front(), outputPorts.front(),concrete.getPredicate(), typeRegistry));
            return ot;
        }
    );

    auto ctx = std::make_unique<llvm::LLVMContext>();
    sierra::odyplan::v2::core::physical::plan::Plan physicalPlan = builder.build(logicalPlan, *ctx);

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
#ifdef ENABLE_LOOPED_SOURCE
    std::thread mainThread([&]() {
#endif
        odyMain();
#ifdef ENABLE_LOOPED_SOURCE
    });

    std::cout << "Running (press Enter to stop:\n";

    std::cin.get();
    atStop.store(true);
    mainThread.join();
#endif

    return 0;
}
