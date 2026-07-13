#ifndef SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_TYPEREGISTRY_H
#define SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_TYPEREGISTRY_H

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <functional>
#include <string>

#include "core/common/identifier/type.h"

namespace sierra::odyplan::v2::core::common::type {
    class TypeRegistry {
    public:
        using LLVMTypeFactory = std::function<llvm::Type*(llvm::LLVMContext&)>;
        using ConstantFactory = std::function<llvm::Value*(llvm::IRBuilder<>&, const std::string&)>;

        static TypeRegistry withDefaults() {
            TypeRegistry registry;
            registry.registerDefaults();
            return registry;
        }

        void registerType(identifier::TypeId id, std::string name, LLVMTypeFactory typeFactory, ConstantFactory constantFactory) {
            mTypeStorage.emplace(id, TypeRegistryEntry {.name=std::move(name), .typeFactory=std::move(typeFactory), .constantFactory=std::move(constantFactory)});
        }

        [[nodiscard]] llvm::Type* resolveType(const identifier::TypeId id, llvm::LLVMContext& ctx) const {
            return get(id).typeFactory(ctx);
        }

        [[nodiscard]] llvm::Value* compileConstant(const identifier::TypeId id, llvm::IRBuilder<>& builder, const std::string& value) const {
            return get(id).constantFactory(builder, value);
        }

    private:
        struct TypeRegistryEntry {
            std::string name;
            LLVMTypeFactory typeFactory;
            ConstantFactory constantFactory;
        };

        std::unordered_map<identifier::TypeId, TypeRegistryEntry> mTypeStorage;

        [[nodiscard]] const TypeRegistryEntry& get(identifier::TypeId id) const {
            const auto iter = mTypeStorage.find(id);
            if (iter == mTypeStorage.end()) {
                throw std::invalid_argument("TypeRegistry: unknown typeId " + std::to_string(id));
            }
            return iter->second;
        }

        void registerDefaults() {
            registerType(
                std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::BOOL), "bool",
                [](llvm::LLVMContext& ctx) {
                    return llvm::Type::getInt1Ty(ctx);
                },
                [](const llvm::IRBuilder<>&b, const std::string& v) {
                return llvm::ConstantInt::get(
                    llvm::Type::getInt1Ty(b.getContext()), std::stoi(v));
                }
                );
            registerType(
                std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::INT8), "int8",
                [](llvm::LLVMContext& ctx) {
                    return llvm::Type::getInt8Ty(ctx);
                },
                [](const llvm::IRBuilder<>&b, const std::string& v) {
                return llvm::ConstantInt::get(
                    llvm::Type::getInt8Ty(b.getContext()), std::stoi(v));
                }
                );
            registerType(
                std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::INT16), "int16",
                [](llvm::LLVMContext& ctx) {
                    return llvm::Type::getInt16Ty(ctx);
                },
                [](const llvm::IRBuilder<>&b, const std::string& v) {
                return llvm::ConstantInt::get(
                    llvm::Type::getInt16Ty(b.getContext()), std::stoi(v));
                }
                );
            registerType(
                std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::INT32), "int32",
                [](llvm::LLVMContext& ctx) {
                    return llvm::Type::getInt32Ty(ctx);
                },
                [](const llvm::IRBuilder<>&b, const std::string& v) {
                return llvm::ConstantInt::get(
                    llvm::Type::getInt32Ty(b.getContext()), std::stoi(v));
                }
                );
            registerType(
                std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::INT64), "int64",
                [](llvm::LLVMContext& ctx) {
                    return llvm::Type::getInt64Ty(ctx);
                },
                [](const llvm::IRBuilder<>&b, const std::string& v) {
                return llvm::ConstantInt::get(
                    llvm::Type::getInt64Ty(b.getContext()), std::stoi(v));
                }
                );
            registerType(
                std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::FLOAT), "float",
                [](llvm::LLVMContext& ctx) {
                    return llvm::Type::getFloatTy(ctx);
                },
                [](const llvm::IRBuilder<>&b, const std::string& v) {
                return llvm::ConstantInt::get(
                    llvm::Type::getFloatTy(b.getContext()), std::stoi(v));
                }
                );
            registerType(
                std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::DOUBLE), "double",
                [](llvm::LLVMContext& ctx) {
                    return llvm::Type::getDoubleTy(ctx);
                },
                [](const llvm::IRBuilder<>&b, const std::string& v) {
                return llvm::ConstantInt::get(
                    llvm::Type::getDoubleTy(b.getContext()), std::stoi(v));
                }
            );
            registerType(
                std::to_underlying(sierra::odyplan::v2::core::common::identifier::Type::STRING), "string",
                [](llvm::LLVMContext& ctx) {
                    return llvm::StructType::get(ctx, {
                        //data_raw
                        llvm::PointerType::get(ctx, 0),
                        //data_length
                        llvm::Type::getInt64Ty(ctx)
                    });
                },
                [](llvm::IRBuilder<>&b, const std::string& v) {
                    auto& ctx = b.getContext();
                    llvm::Value* ptr = b.CreateGlobalString(v, "string_const");
                    llvm::Value* len = llvm::ConstantInt::get(
                        llvm::Type::getInt64Ty(ctx),
                        v.size()
                    );
                    llvm::Type* structType = llvm::StructType::get(ctx, {
                        llvm::PointerType::get(ctx, 0),
                        llvm::Type::getInt64Ty(ctx)
                    });
                    llvm::Value* strStruct = llvm::UndefValue::get(structType);
                    strStruct = b.CreateInsertValue(strStruct, ptr, 0);
                    strStruct = b.CreateInsertValue(strStruct, len, 1);
                    return strStruct;
                }
            );
        }
    };
}

#endif //SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_TYPEREGISTRY_H