#include <core/physical/state/state_registry.h>

#include <ranges>

namespace sierra::odyplan::v3::core::physical::state {
    StateRegistry::StateRegistry(llvm::Module* module, llvm::LLVMContext* context)
        : mModule(module), mContext(context) {
        auto* ptrTy = llvm::PointerType::getUnqual(*context);
        auto* voidTy = llvm::Type::getVoidTy(*context);

        auto* initFnType = llvm::FunctionType::get(voidTy, {}, false);
        mInitFn = llvm::Function::Create(
            initFnType, llvm::Function::ExternalLinkage, "ody_init", *module);
        mInitBuilder = std::make_unique<llvm::IRBuilder<>>(
            llvm::BasicBlock::Create(*context, "entry", mInitFn));

        auto* shutdownFnType = llvm::FunctionType::get(voidTy, {}, false);
        mShutdownFn = llvm::Function::Create(
            shutdownFnType, llvm::Function::ExternalLinkage, "ody_shutdown", *module);
        mShutdownBuilder = std::make_unique<llvm::IRBuilder<>>(
            llvm::BasicBlock::Create(*context, "entry", mShutdownFn));
    }

    StateHandle StateRegistry::createHandle(const std::string& uniqueName,
                                            const std::string& createFnName,
                                            const std::vector<llvm::Value*>& args,
                                            const std::vector<llvm::Type*>& argTypes,
                                            const std::string& destroyFnName) {
        auto* ptrTy = llvm::PointerType::getUnqual(*mContext);
        auto* voidTy = llvm::Type::getVoidTy(*mContext);

        // NOLINTBEGIN(cppcoreguidelines-owning-memory)
        auto* globalVar = new llvm::GlobalVariable(*mModule,
                                                   ptrTy,
                                                   false,
                                                   llvm::GlobalValue::InternalLinkage,
                                                   llvm::ConstantPointerNull::get(ptrTy),
                                                   "ody_state_" + uniqueName);
        // NOLINTEND(cppcoreguidelines-owning-memory)

        auto createFn = mModule->getOrInsertFunction(
            createFnName, llvm::FunctionType::get(ptrTy, argTypes, false));

        llvm::Value* created = mInitBuilder->CreateCall(createFn, args);
        mInitBuilder->CreateStore(created, globalVar);

        auto destroyFn = mModule->getOrInsertFunction(
            destroyFnName, llvm::FunctionType::get(voidTy, {ptrTy}, false));

        mPendingDestroys.push_back({.destroyFn=destroyFn, .statePtr=globalVar});

        return StateHandle{globalVar};
    }

    StateHandle StateRegistry::createPrimitiveHandle(const std::string &uniqueName, llvm::Type *type, llvm::Constant *initialValue) {
        // NOLINTBEGIN(cppcoreguidelines-owning-memory)
        auto* globalVar = new llvm::GlobalVariable(*mModule,
                                               type,
                                               false,
                                               llvm::GlobalValue::InternalLinkage,
                                               initialValue,
                                               "ody_state_" + uniqueName);
        // NOLINTEND(cppcoreguidelines-owning-memory)

        return StateHandle{globalVar};
    }

    void StateRegistry::finish() {
        mInitBuilder->CreateRetVoid();

        auto* ptrTy = llvm::PointerType::getUnqual(*mContext);

        for (auto &[destroyFn, statePtr] : std::ranges::reverse_view(mPendingDestroys)) {
            llvm::Value* loaded = mShutdownBuilder->CreateLoad(ptrTy, statePtr);
            mShutdownBuilder->CreateCall(destroyFn, {loaded});
        }

        mShutdownBuilder->CreateRetVoid();
    }
}  // namespace sierra::odyplan::v3::core::physical::state