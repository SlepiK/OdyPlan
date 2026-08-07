#include <core/physical/state/state_registry.h>

namespace sierra::odyplan::v3::core::physical::state {
    StateRegistry::StateRegistry(llvm::Module* module, llvm::LLVMContext* context)
    : mModule(module), mContext(context){
        auto* ptrTy = llvm::PointerType::getUnqual(*context);
        auto* voidTy = llvm::Type::getVoidTy(*context);

        auto* initFnType = llvm::FunctionType::get(voidTy, {ptrTy}, false);
        mInitFn = llvm::Function::Create(initFnType, llvm::Function::ExternalLinkage, "ody_init", *module);
        mInitFn->getArg(0)->setName("typeRegistry");
        mInitBuilder = std::make_unique<llvm::IRBuilder<>>(llvm::BasicBlock::Create(*context, "entry", mInitFn));

        auto* shutdownFnType = llvm::FunctionType::get(voidTy, {}, false);
        mShutdownFn = llvm::Function::Create(shutdownFnType, llvm::Function::ExternalLinkage, "ody_shutdown", *module);
        mShutdownBuilder = std::make_unique<llvm::IRBuilder<>>(llvm::BasicBlock::Create(*context, "entry", mShutdownFn));
    }

    StateHandle StateRegistry::createHandle(const std::string& uniqueName, const std::string& createFnName,
        const std::vector<llvm::Value*>& extraArgs, const std::vector<llvm::Type*>& extraArgTypes,
        const std::string& destroyFnName) {
        auto* ptrTy = llvm::PointerType::getUnqual(*mContext);
        auto* voidTy = llvm::Type::getVoidTy(*mContext);

        auto* globalVar = new llvm::GlobalVariable(*mModule, ptrTy, false,
            llvm::GlobalValue::InternalLinkage, llvm::ConstantPointerNull::get(ptrTy),
            "ody_state_" + uniqueName);

        std::vector<llvm::Type*> argTypes{ptrTy};
        argTypes.insert(argTypes.end(), extraArgTypes.begin(), extraArgTypes.end());

        auto createFn = mModule->getOrInsertFunction(createFnName, llvm::FunctionType::get(ptrTy, argTypes, false));

        std::vector<llvm::Value*> args{mInitFn->getArg(0)};
        args.insert(args.end(), extraArgs.begin(), extraArgs.end());

        llvm::Value* created = mInitBuilder->CreateCall(createFn, args);
        mInitBuilder->CreateStore(created, globalVar);

        auto destroyFn = mModule->getOrInsertFunction(destroyFnName,
            llvm::FunctionType::get(voidTy, {ptrTy}, false));

        llvm::Value* loaded = mShutdownBuilder->CreateLoad(ptrTy, globalVar);

        mShutdownBuilder->CreateCall(destroyFn, {loaded});

        return StateHandle{globalVar};
    }

    void StateRegistry::finish() {
        mInitBuilder->CreateRetVoid();
        mShutdownBuilder->CreateRetVoid();
    }
}