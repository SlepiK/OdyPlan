#include <core/physical/state/state_registry.h>

namespace sierra::odyplan::v3::core::physical::state {
    StateRegistry::StateRegistry(llvm::Module* module, llvm::LLVMContext* context)
        : mModule(module), mContext(context) {
        auto* ptrTy = llvm::PointerType::getUnqual(*context);
        auto* voidTy = llvm::Type::getVoidTy(*context);

        auto* initFnType = llvm::FunctionType::get(voidTy, {ptrTy}, false);
        mInitFn = llvm::Function::Create(
            initFnType, llvm::Function::ExternalLinkage, "ody_init", *module);
        mInitFn->getArg(0)->setName("runtimeTypeRegistry");
        mInitBuilder = std::make_unique<llvm::IRBuilder<>>(
            llvm::BasicBlock::Create(*context, "entry", mInitFn));

        mRuntimeTypeRegistryGlobal = new llvm::GlobalVariable(
            *module, ptrTy, false, llvm::GlobalValue::PrivateLinkage,
            llvm::ConstantPointerNull::get(ptrTy), "ody_runtime_type_registry");
        mInitBuilder->CreateStore(mInitFn->getArg(0), mRuntimeTypeRegistryGlobal);

        auto* shutdownFnType = llvm::FunctionType::get(voidTy, {}, false);
        mShutdownFn = llvm::Function::Create(
            shutdownFnType, llvm::Function::ExternalLinkage, "ody_shutdown", *module);
        mShutdownBuilder = std::make_unique<llvm::IRBuilder<>>(
            llvm::BasicBlock::Create(*context, "entry", mShutdownFn));
    }

    StateHandle StateRegistry::createHandle(const std::string& uniqueName,
                                            const std::string& createFnName,
                                            const std::vector<llvm::Value*>& extraArgs,
                                            const std::vector<llvm::Type*>& extraArgTypes,
                                            const std::string& destroyFnName) {
        auto* ptrTy = llvm::PointerType::getUnqual(*mContext);
        auto* voidTy = llvm::Type::getVoidTy(*mContext);

        auto* globalVar = new llvm::GlobalVariable(*mModule,
                                                   ptrTy,
                                                   false,
                                                   llvm::GlobalValue::InternalLinkage,
                                                   llvm::ConstantPointerNull::get(ptrTy),
                                                   "ody_state_" + uniqueName);

        std::vector<llvm::Type*> argTypes{ptrTy};
        argTypes.insert(argTypes.end(), extraArgTypes.begin(), extraArgTypes.end());

        auto createFn = mModule->getOrInsertFunction(
            createFnName, llvm::FunctionType::get(ptrTy, argTypes, false));

        std::vector<llvm::Value*> args{mInitFn->getArg(0)};
        args.insert(args.end(), extraArgs.begin(), extraArgs.end());

        llvm::Value* created = mInitBuilder->CreateCall(createFn, args);
        mInitBuilder->CreateStore(created, globalVar);

        auto destroyFn = mModule->getOrInsertFunction(
            destroyFnName, llvm::FunctionType::get(voidTy, {ptrTy}, false));

        mPendingDestroys.push_back({destroyFn, globalVar});

        return StateHandle{globalVar};
    }

    llvm::GlobalVariable* StateRegistry::getRuntimeTypeRegistryGlobal() const {
        return mRuntimeTypeRegistryGlobal;
    }

    void StateRegistry::finish() {
        mInitBuilder->CreateRetVoid();

        auto* ptrTy = llvm::PointerType::getUnqual(*mContext);

        for (auto it = mPendingDestroys.rbegin(); it != mPendingDestroys.rend(); ++it) {
            llvm::Value* loaded = mShutdownBuilder->CreateLoad(ptrTy, it->statePtr);
            mShutdownBuilder->CreateCall(it->destroyFn, {loaded});
        }

        mShutdownBuilder->CreateRetVoid();
    }
}  // namespace sierra::odyplan::v3::core::physical::state