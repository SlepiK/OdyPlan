#include <core/physical/storage/storage.h>

namespace sierra::odyplan::v3::core::physical::storage {
    Storage::Storage(llvm::Function& fn,
                     llvm::LLVMContext& ctx,
                     const types::CodegenTypeRegistry& codegenTypeRegistry)
        : mFn(&fn), mCtx(&ctx), mCodegenTypeRegistry(&codegenTypeRegistry) {}

    llvm::Value* Storage::getNamed(const std::string& name, llvm::Type* llvmType) {
        if (const auto iter = mNamedSlots.find(name); iter != mNamedSlots.end()) {
            return iter->second;
        }

        auto* newSlot = createAlloca(llvmType, "Stg.Named." + name);
        mNamedSlots.emplace(name, newSlot);
        return newSlot;
    }

    llvm::Value* Storage::getNamed(const std::string& name, ValueTypeId valueTypeId) {
        return getNamed(name, mCodegenTypeRegistry->resolveType(valueTypeId, *mCtx));
    }

    llvm::AllocaInst* Storage::createAlloca(llvm::Type* llvmType, const std::string& name) const {
        llvm::IRBuilder<> builder(&mFn->getEntryBlock(), mFn->getEntryBlock().begin());
        return builder.CreateAlloca(llvmType, nullptr, name);
    }
}  // namespace sierra::odyplan::v3::core::physical::storage