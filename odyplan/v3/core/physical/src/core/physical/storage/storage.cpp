#include <core/physical/storage/storage.h>

namespace sierra::odyplan::v3::core::physical::storage {
    Storage::Storage(llvm::Function& fn,
                     llvm::LLVMContext& ctx,
                     const types::ValueTypeRegistry& valueTypeRegistry)
        : mFn(&fn), mCtx(&ctx), mValueTypeRegistry(&valueTypeRegistry) {}

    llvm::Value* Storage::get(ValueTypeId valueTypeId) {
        if (const auto iter = mSlots.find(valueTypeId); iter != mSlots.end()) {
            return iter->second;
        }

        auto* newSlot = createAlloca(valueTypeId);
        mSlots.emplace(valueTypeId, newSlot);

        return newSlot;
    }

    llvm::AllocaInst* Storage::createAlloca(const ValueTypeId valueTypeId) const {
        llvm::Type* llvmType = mValueTypeRegistry->resolveType(valueTypeId, *mCtx);

        llvm::IRBuilder<> builder(&mFn->getEntryBlock(), mFn->getEntryBlock().begin());

        return builder.CreateAlloca(
            llvmType, nullptr, "Storage_ValueTypeId_" + std::to_string(valueTypeId));
    }
}  // namespace sierra::odyplan::v3::core::physical::storage