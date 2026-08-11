#include <core/physical/types/codegen_type_registry.h>

namespace sierra::odyplan::v3::core::physical::types {
    // NOLINTNEXTLINE(readability-function-size)
    CodegenTypeRegistry CodegenTypeRegistry::withBuiltins() {
        using T = shared::types::BuiltinValueType;
        CodegenTypeRegistry registry;

        registry.registerType(
            std::to_underlying(T::BOOL),
            {
                .typeName = "bool",
                .parseFunctionName = "rtParseBool",
                .printFunctionName = "rtOdyPrintInt",
                .llvmTypeFactory = [](llvm::LLVMContext& ctx) {
                    return llvm::Type::getInt1Ty(ctx);
                },
                .constantFactory = [](const llvm::IRBuilder<>& b, const std::string& v) {
                    const bool boolVal = (v == "true" || v == "1");
                    return llvm::ConstantInt::get(llvm::Type::getInt1Ty(b.getContext()), boolVal ? 1 : 0);
                }
            }
        );

        registry.registerType(
            std::to_underlying(T::INT8),
            {
                .typeName = "int8",
                .parseFunctionName = "rtParseInt8",
                .printFunctionName = "rtOdyPrintInt",
                .llvmTypeFactory = [](llvm::LLVMContext& ctx) {
                    return llvm::Type::getInt8Ty(ctx);
                },
                .constantFactory = [](const llvm::IRBuilder<>& b, const std::string& v) {
                    return llvm::ConstantInt::get(llvm::Type::getInt8Ty(b.getContext()), std::stoi(v));
                }
            }
        );

        registry.registerType(
            std::to_underlying(T::INT16),
            {
                .typeName = "int16",
                .parseFunctionName = "rtParseInt16",
                .printFunctionName = "rtOdyPrintInt",
                .llvmTypeFactory = [](llvm::LLVMContext& ctx) {
                    return llvm::Type::getInt16Ty(ctx);
                },
                .constantFactory = [](const llvm::IRBuilder<>& b, const std::string& v) {
                    return llvm::ConstantInt::get(llvm::Type::getInt16Ty(b.getContext()), std::stoi(v));
                }
            }
        );

        registry.registerType(
            std::to_underlying(T::INT32),
            {
                .typeName = "int32",
                .parseFunctionName = "rtParseInt32",
                .printFunctionName = "rtOdyPrintInt",
                .llvmTypeFactory = [](llvm::LLVMContext& ctx) {
                    return llvm::Type::getInt32Ty(ctx);
                },
                .constantFactory = [](const llvm::IRBuilder<>& b, const std::string& v) {
                    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(b.getContext()), std::stoi(v));
                }
            }
        );

        registry.registerType(
            std::to_underlying(T::INT64),
            {
                .typeName = "int64",
                .parseFunctionName = "rtParseInt64",
                .printFunctionName = "rtOdyPrintInt",
                .llvmTypeFactory = [](llvm::LLVMContext& ctx) {
                    return llvm::Type::getInt64Ty(ctx);
                },
                .constantFactory = [](const llvm::IRBuilder<>& b, const std::string& v) {
                    return llvm::ConstantInt::get(llvm::Type::getInt64Ty(b.getContext()), std::stoi(v));
                }
            }
            );

        registry.registerType(
            std::to_underlying(T::FLOAT),
            {
                .typeName = "float",
                .parseFunctionName = "rtParseFloat",
                .printFunctionName = "rtOdyPrintFloat",
                .llvmTypeFactory = [](llvm::LLVMContext& ctx) {
                    return llvm::Type::getFloatTy(ctx);
                },
                .constantFactory = [](const llvm::IRBuilder<>& b, const std::string& v) {
                    return llvm::ConstantFP::get(llvm::Type::getFloatTy(b.getContext()), std::stoi(v));
                }
            }
            );

        registry.registerType(
            std::to_underlying(T::DOUBLE),
            {
                .typeName = "double",
                .parseFunctionName = "rtParseDouble",
                .printFunctionName = "rtOdyPrintDouble",
                .llvmTypeFactory = [](llvm::LLVMContext& ctx) {
                    return llvm::Type::getDoubleTy(ctx);
                },
                .constantFactory = [](const llvm::IRBuilder<>& b, const std::string& v) {
                    return llvm::ConstantFP::get(llvm::Type::getDoubleTy(b.getContext()), std::stoi(v));
                }
            }
            );

        registry.registerType(
            std::to_underlying(T::STRING),
            {
                .typeName = "string",
                .parseFunctionName = "",
                .printFunctionName = "rtOdyPrintString",
                .llvmTypeFactory = [](llvm::LLVMContext& ctx) {
                    return llvm::PointerType::getUnqual(ctx);
                },
                .constantFactory = [](llvm::IRBuilder<>& b, const std::string& v) {
                    return b.CreateGlobalString(v);
                }
            }
        );
        return registry;
    }
}