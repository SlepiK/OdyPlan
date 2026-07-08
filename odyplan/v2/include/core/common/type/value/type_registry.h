#ifndef SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_VALUE_TYPEREGISTRY_H
#define SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_VALUE_TYPEREGISTRY_H

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <functional>
#include <string>

#include "type.h"

namespace sierra::odyplan::v2::core::common::type::value {
    class TypeRegistry {
    public:
        using LLVMTypeFactory = std::function<llvm::Type*(llvm::LLVMContext&)>;
        using ConstantFactory = std::function<llvm::Value*(llvm::IRBuilder<>&, const std::string&)>;

        static TypeRegistry withDefaults() {
            TypeRegistry registry;
            registry.registerDefaults();
            return registry;
        }

        void registerType(TypeId id, std::string name, LLVMTypeFactory typeFactory, ConstantFactory constantFactory) {
            mTypeStorage.emplace(id, TypeRegistryEntry {.name=std::move(name), .typeFactory=std::move(typeFactory), .constantFactory=std::move(constantFactory)});
        }

        [[nodiscard]] llvm::Type* resolveType(const TypeId id, llvm::LLVMContext& ctx) const {
            return get(id).typeFactory(ctx);
        }

        [[nodiscard]] llvm::Value* compileConstant(const TypeId id, llvm::IRBuilder<>& builder, const std::string& value) const {
            return get(id).constantFactory(builder, value);
        }

    private:
        struct TypeRegistryEntry {
            std::string name;
            LLVMTypeFactory typeFactory;
            ConstantFactory constantFactory;
        };

        std::unordered_map<TypeId, TypeRegistryEntry> mTypeStorage;

        [[nodiscard]] const TypeRegistryEntry& get(TypeId id) const {
            const auto iter = mTypeStorage.find(id);
            if (iter == mTypeStorage.end()) {
                throw std::invalid_argument("TypeRegistry: unknown typeId " + std::to_string(id));
            }
            return iter->second;
        }

        void registerDefaults() {
            registerType(
                std::to_underlying(sierra::odyplan::v2::core::common::type::value::Type::INT), "int",
                [](llvm::LLVMContext& ctx) {
                    return llvm::Type::getInt32Ty(ctx);
                },
                [](const llvm::IRBuilder<>&b, const std::string& v) {
                return llvm::ConstantInt::get(
                    llvm::Type::getInt32Ty(b.getContext()), std::stoi(v));
                }
            );
            registerType(
                std::to_underlying(sierra::odyplan::v2::core::common::type::value::Type::STRING), "string",
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

#endif //SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_VALUE_TYPEREGISTRY_H