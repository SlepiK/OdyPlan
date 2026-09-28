#include <core/physical/segment/segment_builder.h>

namespace sierra::odyplan::v3::core::physical::segment {
    llvm::Function* SegmentBuilder::createContinuation(llvm::Module& module, llvm::LLVMContext& ctx, const core::physical::types::CodegenTypeRegistry& codegenTypeRegistry,
    const std::string& name, const std::function<void(physical::segment::SegmentContext&, llvm::Value*)>& bodyFn) {
        auto* voidTy = llvm::Type::getVoidTy(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);

        auto* fn = llvm::Function::Create(
            llvm::FunctionType::get(voidTy, {ptrTy}, false),
            llvm::Function::PrivateLinkage, name, &module);
        fn->getArg(0)->setName("arg");

        auto* entryBB = llvm::BasicBlock::Create(ctx, "entry", fn);
        SegmentContext segCtx(fn, entryBB, ctx, codegenTypeRegistry);

        bodyFn(segCtx, fn->getArg(0));

        segCtx.builder.CreateRetVoid();
        return fn;
    }

    void SegmentBuilder::enqueueToComputeSegment(SegmentContext& segmentCtx, llvm::GlobalVariable* queueSlot,
                                        llvm::Function* processFn, llvm::Value* payload) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);
        auto* voidTy = llvm::Type::getVoidTy(ctx);
        auto* sizeTTy = module->getDataLayout().getIntPtrType(ctx);

        const auto getOrCreateFn = module->getOrInsertFunction(
            "rtComputeSegmentGetOrCreate",
            llvm::FunctionType::get(ptrTy, {ptrTy}, false));
        llvm::Value* segmentHandle = segmentCtx.builder.CreateCall(
            getOrCreateFn, {queueSlot});

        auto* envelopeStructTy = llvm::StructType::create(ctx, {ptrTy, ptrTy});
        const auto& dataLayout = module->getDataLayout();
        const std::uint64_t envelopeByteSize = dataLayout.getTypeAllocSize(envelopeStructTy);

        const auto allocFn = module->getOrInsertFunction(
            "rtFanInAlloc", llvm::FunctionType::get(ptrTy, {sizeTTy}, false));
        llvm::Value* envelopePtr = segmentCtx.builder.CreateCall(allocFn, {llvm::ConstantInt::get(sizeTTy, envelopeByteSize)});

        segmentCtx.builder.CreateStore(processFn, segmentCtx.builder.CreateStructGEP(envelopeStructTy, envelopePtr, 0));
        segmentCtx.builder.CreateStore(payload, segmentCtx.builder.CreateStructGEP(envelopeStructTy, envelopePtr, 1));

        const auto enqueueFn = module->getOrInsertFunction(
            "rtComputeSegmentEnqueue",
            llvm::FunctionType::get(voidTy, {ptrTy, ptrTy}, false));
        segmentCtx.builder.CreateCall(enqueueFn, {segmentHandle, envelopePtr});
    }
}