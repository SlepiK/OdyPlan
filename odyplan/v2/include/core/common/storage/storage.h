#ifndef SIERRA_ODYPLAN_V2_CORE_COMMON_STORAGE_STORAGE_H
#define SIERRA_ODYPLAN_V2_CORE_COMMON_STORAGE_STORAGE_H
#include <unordered_map>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>

#include "core/common/identifier/type.h"
#include "core/common/type/type_registry.h"

namespace sierra::odyplan::v2::core::common::storage {
    class Storage {
    public:
        Storage(llvm::Function& fn, llvm::LLVMContext& ctx, const type::TypeRegistry& typeRegistry)
            : mFn(&fn), mCtx(&ctx), mTypeRegistry(&typeRegistry) {
        }

        llvm::Value* get(identifier::TypeId typeId) {
            if (const auto iter = mSlots.find(typeId); iter != mSlots.end()) {
                return iter->second;
            }

            auto* newSlot = createAlloca(typeId);
            mSlots.emplace(typeId, newSlot);

            return newSlot;
        }
    private:
        llvm::Function* mFn;
        llvm::LLVMContext* mCtx;
        const type::TypeRegistry* mTypeRegistry;

        std::unordered_map<identifier::TypeId, llvm::AllocaInst*> mSlots;

        llvm::AllocaInst* createAlloca(const identifier::TypeId typeId) const {
            llvm::Type* llvmType = mTypeRegistry->resolveType(typeId, *mCtx);

            llvm::IRBuilder<> entryBuilder(&mFn->getEntryBlock(), mFn->getEntryBlock().begin());

            return entryBuilder.CreateAlloca(llvmType, nullptr, "Stg.Typ." + std::to_string(typeId));
        }
    };
}

#endif //SIERRA_ODYPLAN_V2_CORE_COMMON_STORAGE_STORAGE_H