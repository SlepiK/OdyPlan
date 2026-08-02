#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_STORAGES_STORAGE_H
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_STORAGES_STORAGE_H

#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>

#include <core/physical/types/value_type_registry.h>

#include <core/shared/types/value_type_id.h>

namespace sierra::odyplan::v3::core::physical::storages {
    class Storage {
    public:
    	using ValueTypeId = shared::types::ValueTypeId;

        Storage(llvm::Function& fn, llvm::LLVMContext& ctx, const types::ValueTypeRegistry& valueTypeRegistry);

        [[nodiscard]] llvm::Value* get(ValueTypeId valueTypeId);

    private:
        llvm::Function* mFn;
        llvm::LLVMContext* mCtx;
        const types::ValueTypeRegistry* mValueTypeRegistry;

        std::unordered_map<ValueTypeId, llvm::AllocaInst*> mSlots;

        [[nodiscard]] llvm::AllocaInst* createAlloca(const ValueTypeId valueTypeId) const;
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_PHYSICAL_STORAGES_STORAGE_H