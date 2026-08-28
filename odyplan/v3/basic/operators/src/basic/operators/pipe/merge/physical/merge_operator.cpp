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
            "rtMergeWatermarkInit",
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
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);

        llvm::Value* statePtr = segmentCtx.builder.CreateLoad(ptrTy, mWatermarkState.var);

        llvm::FunctionType* fnType = llvm::FunctionType::get(i32Ty, {ptrTy, i64Ty, i64Ty, ptrTy}, false);
        llvm::FunctionCallee fn = module->getOrInsertFunction("rtMergeUpdateWatermark", fnType);

        llvm::Value* outSlot = segmentCtx.builder.CreateAlloca(i64Ty, nullptr, "watermark_out");

        llvm::Value* sEmit = segmentCtx.builder.CreateCall(fn, {statePtr, llvm::ConstantInt::get(i64Ty, cmpIndex),
            punctuation.watermark, outSlot}, "should_emit");

        llvm::Value* condition = segmentCtx.builder.CreateICmpNE(sEmit, llvm::ConstantInt::get(i32Ty, 0));

        auto* wFn = segmentCtx.builder.GetInsertBlock()->getParent();
        auto* emitBB = llvm::BasicBlock::Create(ctx, "merge_emit_watermark", wFn);
        auto* mergeBB = llvm::BasicBlock::Create(ctx, "merge_watermark_merge", wFn);

        segmentCtx.builder.CreateCondBr(condition,emitBB,mergeBB);
        segmentCtx.builder.SetInsertPoint(emitBB);
        llvm::Value* newOpGlobWatermark = segmentCtx.builder.CreateLoad(i64Ty, outSlot);
        const core::physical::data::PunctuationPackage outgoingPunctuation{
            .watermark = newOpGlobWatermark
        };
        consumers.front().consumePunctuation(segmentCtx, outgoingPunctuation);
        segmentCtx.builder.CreateBr(mergeBB);

        segmentCtx.builder.SetInsertPoint(mergeBB);
    }

}  // namespace sierra::odyplan::v3::basic::operators::pipe::merge::physical