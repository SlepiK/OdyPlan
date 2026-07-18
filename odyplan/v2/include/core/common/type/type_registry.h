#ifndef SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_TYPEREGISTRY_H
#define SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_TYPEREGISTRY_H

#include <cmath>
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

        void registerType(identifier::TypeId id, std::string name, const std::size_t byteSize, LLVMTypeFactory typeFactory, ConstantFactory constantFactory) {
            mTypeStorage.emplace(id, TypeRegistryEntry {.name=std::move(name), .byteSize = byteSize, .typeFactory=std::move(typeFactory), .constantFactory=std::move(constantFactory)});
        }

        [[nodiscard]] llvm::Type* resolveType(const identifier::TypeId id, llvm::LLVMContext& ctx) const {
            return get(id).typeFactory(ctx);
        }

        [[nodiscard]] llvm::Value* compileConstant(const identifier::TypeId id, llvm::IRBuilder<>& builder, const std::string& value) const {
            return get(id).constantFactory(builder, value);
        }

        [[nodiscard]] std::size_t getByteSize(const identifier::TypeId id) const {
            return get(id).byteSize;
        }

    private:
        struct TypeRegistryEntry {
            std::string name;
            std::size_t byteSize;
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
                sizeof(bool),
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
                sizeof(std::int8_t),
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
                sizeof(std::int16_t),
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
                sizeof(std::int32_t),
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
                sizeof(std::int64_t),
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
                sizeof(std::float_t),
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
                sizeof(std::double_t),
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
                sizeof(const char*),
                [](llvm::LLVMContext& ctx) {
                    return llvm::PointerType::getUnqual(ctx);
                },
                [](llvm::IRBuilder<>&b, const std::string& v) {
                    return b.CreateGlobalString(v);
                }
            );
        }
    };
}

#endif //SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_TYPEREGISTRY_H