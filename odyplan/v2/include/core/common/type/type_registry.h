#ifndef SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_TYPEREGISTRY_H
#define SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_TYPEREGISTRY_H

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <functional>
#include <string>

#include "type.h"

namespace sierra::odyplan::v2::core::common::type {
    class TypeRegistry {
    public:
        using LLVMTypeFactory = std::function<llvm::Type*(llvm::LLVMContext&)>;
        using ConstantFactory = std::function<llvm::Value*(llvm::IRBuilder<>&, const std::string&)>;

        void registerType(TypeId id, std::string name, LLVMTypeFactory typeFactory, ConstantFactory constantFactory) {
            mTypeStorage.emplace(id, TypeRegistryEntry {std::move(name), std::move(typeFactory), std::move(constantFactory)});
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
    };
}

#endif //SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_TYPEREGISTRY_H