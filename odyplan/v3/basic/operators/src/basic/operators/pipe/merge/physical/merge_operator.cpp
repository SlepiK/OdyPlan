#include <basic/operators/pipe/merge/physical/merge_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::merge::physical {
    MergeMergeOperator::Operator(core::physical::port::Port inputPort, core::physical::port::Port outputPort)
            : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)},
                                core::physical::port::Ports{std::move(outputPort)}) {
    }

    void MergeOperator::declareState(core::physical::state::StateRegistry &stateRegistry, llvm::Module & /*module*/,
                      llvm::LLVMContext & /*context*/) {
        mStateHandle = stateRegistry.createHandle(
            "ody_merge_" + std::to_string(getId()),
            "rtCreateMergeLock",
            {},
            {},
            "rtDestroyMergeLock");
    }

    void MergeOperator::consume(core::physical::segment::SegmentContext &segmentCtx,
                 core::shared::identifiers::PortId /*inputPort*/,
                 const core::physical::data::DataPackage &tuple,
                 const core::physical::segment::SegmentDataConsumers &consumers) {
        llvm::LLVMContext &ctx = segmentCtx.builder.getContext();
        llvm::Module *module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto *voidTy = llvm::Type::getVoidTy(ctx);
        auto *ptrTy = llvm::PointerType::getUnqual(ctx);

        llvm::Value *state = loadStatePtr(segmentCtx);

        auto lockFn = module->getOrInsertFunction("rtMergeLock", llvm::FunctionType::get(voidTy, {ptrTy}, false));
        segmentCtx.builder.CreateCall(lockFn, {state});

        consumers.at(0)(segmentCtx, tuple);

        auto unlockFn = module->getOrInsertFunction("rtMergeUnlock",
                                                    llvm::FunctionType::get(voidTy, {ptrTy}, false));
        segmentCtx.builder.CreateCall(unlockFn, {state});
    }

    llvm::Value *MergeOperator::loadStatePtr(core::physical::segment::SegmentContext &segmentCtx) const {
        auto *ptrTy = llvm::PointerType::getUnqual(segmentCtx.builder.getContext());
        return segmentCtx.builder.CreateLoad(ptrTy, mStateHandle.var);
    }
}