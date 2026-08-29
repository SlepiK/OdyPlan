#include <basic/operators/pipe/merge/physical/merge_operator.h>

#include "spdlog/fmt/bundled/base.h"

namespace sierra::odyplan::v3::basic::operators::pipe::merge::physical {
    MergeOperator::MergeOperator(core::physical::port::Port inputPort,
                                 core::physical::port::Port outputPort)
        : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)},
                            core::physical::port::Ports{std::move(outputPort)}) {}

    void MergeOperator::consume(core::physical::segment::SegmentContext& segmentCtx,
                                core::shared::identifiers::PortId /*inputPort*/,
                                const core::physical::data::DataPackage& tuple,
                                const core::physical::segment::SegmentDataConsumers& consumers) {
        consumers.front().consume(segmentCtx, tuple);
    }

    void MergeOperator::declareState(core::physical::state::StateRegistry &stateRegistry, llvm::Module &module, llvm::LLVMContext &context) {
        auto* i64Ty = llvm::Type::getInt64Ty(context);
        mWatermarkState = stateRegistry.createHandle(
            "merge_watermark_" +std::to_string(getPId()),
            "rtMergeWatermarkCreate",
            {llvm::ConstantInt::get(i64Ty, static_cast<std::int64_t>(sizeOfConnectedPorts()))},
            {i64Ty},
            "rtMergeWatermarkDestroy");
    }

    void MergeOperator::consumePunctuation(core::physical::segment::SegmentContext &segmentCtx,
                            core::shared::identifiers::PortId inputPort,
                            const core::physical::data::PunctuationPackage &punctuation,
                            const core::physical::segment::SegmentDataConsumers &consumers) {
        const auto cmpIndex = compactInputIndex(inputPort);

        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto* i64Ty = llvm::Type::getInt64Ty(ctx);
        auto* i32Ty = llvm::Type::getInt32Ty(ctx);
        auto* i8Ty = llvm::Type::getInt8Ty(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);

        llvm::Value* statePtr = segmentCtx.builder.CreateLoad(ptrTy, mWatermarkState.var);

        llvm::Value* isFinal = segmentCtx.builder.CreateZExt(punctuation.isFinal, i8Ty);

        llvm::FunctionType* fnType = llvm::FunctionType::get(i8Ty, {ptrTy, i64Ty, i64Ty, i8Ty, ptrTy, ptrTy}, false);
        llvm::FunctionCallee fn = module->getOrInsertFunction("rtMergeUpdateWatermark", fnType);

        llvm::Value* outWatermarkSlot = segmentCtx.builder.CreateAlloca(i64Ty, nullptr);
        llvm::Value* outIsFinalSlot = segmentCtx.builder.CreateAlloca(i64Ty, nullptr);

        llvm::Value* sEmit = segmentCtx.builder.CreateCall(fn, {statePtr, llvm::ConstantInt::get(i64Ty, cmpIndex),
            punctuation.watermark, isFinal, outWatermarkSlot, outIsFinalSlot}, "should_emit");

        llvm::Value* condition = segmentCtx.builder.CreateICmpNE(sEmit, llvm::ConstantInt::get(i8Ty, 0));

        auto* wFn = segmentCtx.builder.GetInsertBlock()->getParent();
        auto* emitBB = llvm::BasicBlock::Create(ctx, "merge_emit_watermark", wFn);
        auto* mergeBB = llvm::BasicBlock::Create(ctx, "merge_watermark_merge", wFn);

        segmentCtx.builder.CreateCondBr(condition,emitBB,mergeBB);

        segmentCtx.builder.SetInsertPoint(emitBB);
        llvm::Value* newOpGlobWatermark = segmentCtx.builder.CreateLoad(i64Ty, outWatermarkSlot);
        llvm::Value* newOpGlobIsFinal = segmentCtx.builder.CreateLoad(i8Ty, outIsFinalSlot);
        llvm::Value* newIsFinal = segmentCtx.builder.CreateICmpNE(
            newOpGlobIsFinal, llvm::ConstantInt::get(i8Ty, 0));

        const core::physical::data::PunctuationPackage outgoingPunctuation{
            .watermark = newOpGlobWatermark,
            .isFinal = newIsFinal
        };
        consumers.front().consumePunctuation(segmentCtx, outgoingPunctuation);
        segmentCtx.builder.CreateBr(mergeBB);

        segmentCtx.builder.SetInsertPoint(mergeBB);
    }

}  // namespace sierra::odyplan::v3::basic::operators::pipe::merge::physical