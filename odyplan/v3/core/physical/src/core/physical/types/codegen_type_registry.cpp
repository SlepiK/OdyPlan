#include <core/physical/types/codegen_type_registry.h>
#include <mutex>
#include <utility>

namespace sierra::odyplan::v3::core::physical::types {
    CodegenTypeRegistry &CodegenTypeRegistry::instance() {
        static CodegenTypeRegistry instance;
        static std::once_flag initFl;
        std::call_once(initFl, []() {
            instance.initializeBuiltins();
        });
        return instance;
    }

    // NOLINTNEXTLINE(readability-function-size)
    void CodegenTypeRegistry::initializeBuiltins() {
        using T = shared::types::BuiltinValueType;

        registerType(
            std::to_underlying(T::BOOL),
            {.typeName = "bool",
             .parseFunctionName = "rtParseBool",
             .llvmTypeFactory = [](llvm::LLVMContext& ctx) { return llvm::Type::getInt1Ty(ctx); },
             .constantFactory =
                 [](const llvm::IRBuilder<>& b, const std::string& v) {
                     const bool boolVal = (v == "true" || v == "1");
                     return llvm::ConstantInt::get(llvm::Type::getInt1Ty(b.getContext()),
                                                   boolVal ? 1 : 0);
                 }});

        registerType(
            std::to_underlying(T::INT8),
            {.typeName = "int8",
             .parseFunctionName = "rtParseInt8",
             .llvmTypeFactory = [](llvm::LLVMContext& ctx) { return llvm::Type::getInt8Ty(ctx); },
             .constantFactory =
                 [](const llvm::IRBuilder<>& b, const std::string& v) {
                     return llvm::ConstantInt::get(llvm::Type::getInt8Ty(b.getContext()),
                                                   std::stoi(v));
                 }});

        registerType(
            std::to_underlying(T::INT16),
            {.typeName = "int16",
             .parseFunctionName = "rtParseInt16",
             .llvmTypeFactory = [](llvm::LLVMContext& ctx) { return llvm::Type::getInt16Ty(ctx); },
             .constantFactory =
                 [](const llvm::IRBuilder<>& b, const std::string& v) {
                     return llvm::ConstantInt::get(llvm::Type::getInt16Ty(b.getContext()),
                                                   std::stoi(v));
                 }});

        registerType(
            std::to_underlying(T::INT32),
            {.typeName = "int32",
             .parseFunctionName = "rtParseInt32",
             .llvmTypeFactory = [](llvm::LLVMContext& ctx) { return llvm::Type::getInt32Ty(ctx); },
             .constantFactory =
                 [](const llvm::IRBuilder<>& b, const std::string& v) {
                     return llvm::ConstantInt::get(llvm::Type::getInt32Ty(b.getContext()),
                                                   std::stoi(v));
                 }});

        registerType(
            std::to_underlying(T::INT64),
            {.typeName = "int64",
             .parseFunctionName = "rtParseInt64",
             .llvmTypeFactory = [](llvm::LLVMContext& ctx) { return llvm::Type::getInt64Ty(ctx); },
             .constantFactory =
                 [](const llvm::IRBuilder<>& b, const std::string& v) {
                     return llvm::ConstantInt::get(llvm::Type::getInt64Ty(b.getContext()),
                                                   std::stoi(v));
                 }});

        registerType(
            std::to_underlying(T::FLOAT),
            {.typeName = "float",
             .parseFunctionName = "rtParseFloat",
             .llvmTypeFactory = [](llvm::LLVMContext& ctx) { return llvm::Type::getFloatTy(ctx); },
             .constantFactory =
                 [](const llvm::IRBuilder<>& b, const std::string& v) {
                     return llvm::ConstantFP::get(llvm::Type::getFloatTy(b.getContext()),
                                                  std::stoi(v));
                 }});

        registerType(
            std::to_underlying(T::DOUBLE),
            {.typeName = "double",
             .parseFunctionName = "rtParseDouble",
             .llvmTypeFactory = [](llvm::LLVMContext& ctx) { return llvm::Type::getDoubleTy(ctx); },
             .constantFactory =
                 [](const llvm::IRBuilder<>& b, const std::string& v) {
                     return llvm::ConstantFP::get(llvm::Type::getDoubleTy(b.getContext()),
                                                  std::stoi(v));
                 }});

        registerType(
            std::to_underlying(T::STRING),
            {.typeName = "string",
             .parseFunctionName = "rtParseString",
             .llvmTypeFactory =
                 [](llvm::LLVMContext& ctx) { return llvm::PointerType::getUnqual(ctx); },
             .constantFactory = [](llvm::IRBuilder<>& b,
                                   const std::string& v) { return b.CreateGlobalString(v); }});


        registerType(
            std::to_underlying(T::TIMESTAMP),
            {.typeName = "timestamp",
             .parseFunctionName = "rtParseInt64",
             .llvmTypeFactory = [](llvm::LLVMContext& ctx) { return llvm::Type::getInt64Ty(ctx); },
             .constantFactory =
                 [](const llvm::IRBuilder<>& b, const std::string& v) {
                     return llvm::ConstantInt::get(llvm::Type::getInt64Ty(b.getContext()),
                                                   std::stoi(v));
                 }});
    }
}  // namespace sierra::odyplan::v3::core::physical::types
