#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_STORAGE_STORAGE_H
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_STORAGE_STORAGE_H

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>

#include <core/physical/types/codegen_type_registry.h>
#include <core/shared/types/value_type_id.h>

namespace sierra::odyplan::v3::core::physical::storage {
    class Storage {
    public:
        using ValueTypeId = shared::types::ValueTypeId;

        Storage(llvm::Function& fn,
                llvm::LLVMContext& ctx,
                const types::CodegenTypeRegistry& codegenTypeRegistry);

        [[nodiscard]] llvm::Value* getNamed(const std::string& name, llvm::Type* llvmType);
        [[nodiscard]] llvm::Value* getNamed(const std::string& name, ValueTypeId valueTypeId);

    private:
        llvm::Function* mFn;
        llvm::LLVMContext* mCtx;
        const types::CodegenTypeRegistry* mCodegenTypeRegistry;

        std::unordered_map<std::string, llvm::AllocaInst*> mNamedSlots;

        [[nodiscard]] llvm::AllocaInst* createAlloca(llvm::Type* llvmType, const std::string& name) const;
    };
}  // namespace sierra::odyplan::v3::core::physical::storage

#endif  // SIERRA_ODYPLAN_V3_CORE_PHYSICAL_STORAGE_STORAGE_H