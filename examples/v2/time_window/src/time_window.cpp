#include <iostream>
#include <llvm/Support/TargetSelect.h>

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Support/raw_os_ostream.h>

#include "common/operator/sink/print_operator.h"
#include "core/logical/plan/plan.h"

#include "core/builder/plan_builder.h"

#include <atomic>
#include <sys/socket.h>

#include "common/io/io_registry.h"
#include "common/operator/pipe/select_operator.h"
#include "../../../../odyplan/v2/include/common/operator/pipe/window/count_window_operator.h"
#include "common/operator/pipe/window/time_window_operator.h"
#include "common/operator/source/read_operator.h"
#include "common/window/window_buffer.h"
#include "common/window/count/count_window_buffer.h"
#include "common/window/time/time_window_buffer.h"

inline std::atomic<bool> atStop{false};

inline std::vector<std::unique_ptr<sierra::odyplan::v2::common::io::IIOSource>> gSources;

inline sierra::odyplan::v2::common::io::IOSourceId registerSource(std::unique_ptr<sierra::odyplan::v2::common::io::IIOSource> source) {
    gSources.push_back(std::move(source));
    return gSources.size() - 1;
}

inline std::vector<std::unique_ptr<sierra::odyplan::v2::common::window::IWindowBuffer>> gWindows;

inline std::uint64_t registerWindow(std::unique_ptr<sierra::odyplan::v2::common::window::IWindowBuffer> window) {
    gWindows.push_back(std::move(window));
    return gWindows.size() - 1;
}

inline void shutdownWindows() {
    for (const auto &window : gWindows) {
        window->close();
    }
    gWindows.clear();
}

extern "C" {

    void rtWindowBeginRow(const std::uint64_t handle) {
        gWindows.at(handle)->begin();
    }

    void rtWindowPush(const std::uint64_t handle, const std::uint64_t fieldIndex, const void* rawData) {
        gWindows.at(handle)->push(fieldIndex, rawData);
    }

    void rtWindowEndRow(const std::uint64_t handle, const std::int64_t timestamp) {
        gWindows.at(handle)->end(std::chrono::sys_time<std::chrono::milliseconds>(std::chrono::milliseconds(timestamp)));
    }

    void rtWindowCompute(const std::uint64_t handle) {
        gWindows.at(handle)->compute();
    }

    void rtWindowFlush(const std::uint64_t handle) {
        gWindows.at(handle)->flush();
    }

    std::uint64_t rtWindowGetCount(const std::uint64_t handle) {
        return gWindows.at(handle)->getWindowCount();
    }

    std::uint64_t rtWindowGetRowCountInWindow(const std::uint64_t handle, const std::uint64_t windowIndex) {
        return gWindows.at(handle)->getRowCountInWindow(windowIndex);
    }

    const void* rtWindowGetData(const std::uint64_t handle, const uint64_t windowIndex, const uint64_t rowIndex, const uint64_t fieldIndex) {
        return static_cast<const void*>(gWindows.at(handle)->getData(windowIndex, rowIndex, fieldIndex));
    }

    void rtEnd() {
    }

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

struct WindowShutdownGuard {
    ~WindowShutdownGuard() {
        shutdownWindows();
    }
};

int main() {
    sierra::odyplan::v2::core::logical::plan::Plan logicalPlan;

    std::cout << "Building Logical Plan...\n";

    auto sourceOutput = sierra::odyplan::v2::core::logical::port::Port {
        sierra::odyplan::v2::core::logical::port::PortDescriptor {.typeId= std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::INT64),.typeJsonName="int", .fieldName = "id"},
        sierra::odyplan::v2::core::logical::port::PortDescriptor {.typeId= std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::DOUBLE),.typeJsonName="double", .fieldName = "value"},
        sierra::odyplan::v2::core::logical::port::PortDescriptor {.typeId= std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::STRING),.typeJsonName="string", .fieldName = "name"}
    };

    auto sourceId = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v2::common::opr::source::logical::ReadOperator>(sourceOutput, sierra::odyplan::v2::common::io::IOFormat::JSON, nlohmann::json{{"path", "read.json"}}));

    auto pipeId = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v2::common::opr::pipe::logical::TimeWindowOperator>(
        sierra::odyplan::v2::common::window::time::TimeWindowConfig{
            .type = sierra::odyplan::v2::common::window::WindowType::SLIDING,
            .size = std::chrono::milliseconds(1000),
            .slide = std::chrono::milliseconds(1000),
            .timestampIndex = 0}
    ));

    auto sinkId = logicalPlan.addOperator(std::make_unique<sierra::odyplan::v2::common::opr::sink::logical::PrintOperator>(
        sierra::odyplan::v2::common::opr::sink::logical::PrintOperator::PortFieldIndexV {0,1,2}
    ));

    logicalPlan.connect(
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=sourceId,.portId=0},
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=pipeId,.portId=0});

    logicalPlan.connect(
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=pipeId,.portId=0},
        sierra::odyplan::v2::core::common::port::PortRef{.operatorId=sinkId,.portId=0});

    std::cout << "Logical Plan: OK\n";

    std::cout << "Building Physical Plan...\n";
    auto typeRegistry = sierra::odyplan::v2::core::common::type::TypeRegistry::withDefaults();
    auto ioRegistry = sierra::odyplan::v2::common::io::IORegistry::withDefaults();

    WindowShutdownGuard windowShutdownGuard;

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
        sierra::odyplan::v2::common::opr::pipe::logical::TimeWindowOperator::TYPE_ID,
        [&typeRegistry](const sierra::odyplan::v2::core::logical::opr::IOperator& op,
            const sierra::odyplan::v2::core::physical::port::Ports& inputPorts,
            const sierra::odyplan::v2::core::physical::port::Ports& outputPorts) -> sierra::odyplan::v2::core::physical::transformation::OperatorTransformation {
            const auto& concrete = static_cast<const sierra::odyplan::v2::common::opr::pipe::logical::TimeWindowOperator&>(op);
            const auto& config = concrete.getWindowConfig();

            auto windowHandle = registerWindow(std::make_unique<sierra::odyplan::v2::common::window::time::TimeWindowBuffer>(config, concrete.getOutputPorts().at(0), typeRegistry));

            sierra::odyplan::v2::core::physical::transformation::OperatorTransformation ot;
            ot.oprs.push_back(std::make_unique<sierra::odyplan::v2::common::opr::pipe::physical::TimeWindowOperator>(inputPorts.front(), outputPorts.front(), windowHandle, config.timestampIndex));
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

    auto ctx = std::make_unique<llvm::LLVMContext>();
    sierra::odyplan::v2::core::physical::plan::Plan physicalPlan = builder.build(logicalPlan, *ctx);

    std::cout << "Physical Plan: OK\n";
    std::cout << "Building LLVM-IR...\n";

    auto module = std::make_unique<llvm::Module>("example_window",*ctx);
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


    symbolMap[jit->mangleAndIntern("rtWindowGetRowCountInWindow")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtWindowGetRowCountInWindow),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtWindowGetData")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtWindowGetData),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtWindowBeginRow")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtWindowBeginRow),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtWindowGetCount")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtWindowGetCount),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtWindowCompute")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtWindowCompute),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtWindowPush")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtWindowPush),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtWindowEndRow")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtWindowEndRow),llvm::JITSymbolFlags::Exported);
    symbolMap[jit->mangleAndIntern("rtWindowFlush")] = llvm::orc::ExecutorSymbolDef(llvm::orc::ExecutorAddr::fromPtr(&rtWindowFlush),llvm::JITSymbolFlags::Exported);

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
