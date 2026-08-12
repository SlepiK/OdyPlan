#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_TYPES_CODEGEN_TYPE_REGISTRY_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_TYPES_CODEGEN_TYPE_REGISTRY_HPP
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include "core/shared/types/value_type_id.h"

namespace sierra::odyplan::v3::core::physical::types {
    class CodegenTypeRegistry {
    public:
        using LLVMTypeFactory = std::function<llvm::Type*(llvm::LLVMContext &)>;
        using ConstantFactory = std::function<llvm::Value*(llvm::IRBuilder<> &, const std::string &)>;

        struct Entry {
            std::string typeName;
            std::string parseFunctionName;
            std::string printFunctionName;
            LLVMTypeFactory llvmTypeFactory;
            ConstantFactory constantFactory;
        };

        void registerType(const shared::types::ValueTypeId id, Entry entry) {
            mStorage.emplace(id, std::move(entry));
        }

        [[nodiscard]] llvm::Type *resolveType(const shared::types::ValueTypeId id, llvm::LLVMContext &ctx) const {
            return get(id).llvmTypeFactory(ctx);
        }

        [[nodiscard]] llvm::Value *compileConstant(const shared::types::ValueTypeId id, llvm::IRBuilder<> &builder,
                                                   const std::string &value) const {
            return get(id).constantFactory(builder, value);
        }

        [[nodiscard]] llvm::Value *emitParseCall(const shared::types::ValueTypeId id, llvm::Value *strPtr,
                                                 llvm::IRBuilder<> &builder, llvm::Module &module) const {
            const auto &entry = get(id);

            if (entry.parseFunctionName.empty()) {
                throw std::invalid_argument(
                    "CodegenTypeRegistry: no parse function registered for typeid \"" + std::to_string(id) + "\"");
            }

            const auto function = module.getOrInsertFunction(
                entry.parseFunctionName,
                llvm::FunctionType::get(
                    entry.llvmTypeFactory(builder.getContext()),
                    {llvm::PointerType::getUnqual(builder.getContext())},
                    false
                )
            );

            return builder.CreateCall(function, {strPtr});
        }

        [[nodiscard]] const std::string &typeName(const shared::types::ValueTypeId id) const {
            return get(id).typeName;
        }

        static CodegenTypeRegistry withBuiltins();

    private:
        std::unordered_map<shared::types::ValueTypeId, Entry> mStorage;

        [[nodiscard]] const Entry &get(const shared::types::ValueTypeId id) const {
            const auto it = mStorage.find(id);
            if (it == mStorage.end()) {
                throw std::invalid_argument("CodegenTypeRegistry: unknown typeid \"" + std::to_string(id) + "\"");
            }
            return it->second;
        }
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_PHYSICAL_TYPES_CODEGEN_TYPE_REGISTRY_HPP
