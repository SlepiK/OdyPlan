#include <basic/operators/pipe/union/physical/union_operator.h>

#include "spdlog/fmt/fmt.h"

namespace sierra::odyplan::v3::basic::operators::pipe::union_::physical {
    UnionOperator::UnionOperator(core::physical::port::Port inputPort,
                                 core::physical::port::Port outputPort,
                                 core::logical::port::Port schema, const bool outOfOrder)
        : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)},
                            core::physical::port::Ports{std::move(outputPort)}), mOutOfOrder(outOfOrder),
          mSchema(std::move(schema)) {
    }

    void UnionOperator::consume(core::physical::segment::SegmentContext &segmentCtx,
                                core::shared::identifiers::PortId inputPort,
                                const core::physical::data::DataPackage &tuple,
                                const core::physical::segment::SegmentDataConsumers &consumers) {
        llvm::LLVMContext &ctx = segmentCtx.builder.getContext();
        llvm::Module *module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto *voidTy = llvm::Type::getVoidTy(ctx);
        auto *ptrTy = llvm::PointerType::getUnqual(ctx);
        auto *i64Ty = llvm::Type::getInt64Ty(ctx);

        llvm::Value *bufferStatePtr = segmentCtx.builder.CreateLoad(ptrTy, mUnionBufferState.var);

        if (mOutOfOrder) {
            const auto lockFn = module->getOrInsertFunction("rtUnionLock", llvm::FunctionType::get(voidTy, {ptrTy}, false));
            segmentCtx.builder.CreateCall(lockFn, {bufferStatePtr});

            const auto beginFn = module->getOrInsertFunction("rtUnionBufferBegin",
                                                             llvm::FunctionType::get(voidTy, {ptrTy}, false));
            segmentCtx.builder.CreateCall(beginFn, {bufferStatePtr});

            const auto pushFn = module->getOrInsertFunction("rtUnionBufferPush",
                                                            llvm::FunctionType::get(
                                                                voidTy, {ptrTy, i64Ty, ptrTy}, false));

            for (std::size_t i = 0; i < mSchema.size(); ++i) {
                auto *slot = segmentCtx.storage.getNamed("union_slot." + std::to_string(getPId()),
                                                         mSchema.at(i).valueTypeId);
                segmentCtx.builder.CreateStore(tuple.at(i), slot);
                segmentCtx.builder.CreateCall(pushFn, {bufferStatePtr, llvm::ConstantInt::get(i64Ty, i), slot});
            }

            const auto endFn = module->getOrInsertFunction("rtUnionBufferEnd",
                                                           llvm::FunctionType::get(
                                                               voidTy, {ptrTy, i64Ty, i64Ty, i64Ty, i64Ty, i64Ty},
                                                               false));

            llvm::Value *inputPortVal = llvm::ConstantInt::get(i64Ty,
                                                               static_cast<std::uint64_t>(compactInputIndex(
                                                                   inputPort)));
            llvm::Value *startTsVal = tuple.timeInterval.getStart().getPoint();
            llvm::Value *endTsVal = tuple.timeInterval.getEnd().getPoint();
            llvm::Value *startTbVal = tuple.timeInterval.getStart().getTiBreaker();
            llvm::Value *endTbVal = tuple.timeInterval.getEnd().getTiBreaker();

            segmentCtx.builder.CreateCall(endFn, {
                                              bufferStatePtr,
                                              inputPortVal,
                                              startTsVal,
                                              endTsVal,
                                              startTbVal,
                                              endTbVal
                                          });

            const auto unlockFn = module->getOrInsertFunction("rtUnionUnlock", llvm::FunctionType::get(voidTy, {ptrTy}, false));
            segmentCtx.builder.CreateCall(unlockFn, {bufferStatePtr});
        } else {
            consumers.front().consume(segmentCtx, tuple);
        }
    }

    void UnionOperator::declareState(core::physical::state::StateRegistry &stateRegistry, llvm::Module &module, llvm::LLVMContext &context) {
        auto* i64Ty = llvm::Type::getInt64Ty(context);
        auto* i8Ty = llvm::Type::getInt8Ty(context);
        auto* ptrTy = llvm::PointerType::getUnqual(context);

        mWatermarkState = stateRegistry.createHandle(
            "union_watermark_" +std::to_string(getPId()),
            "rtUnionWatermarkCreate",
            {llvm::ConstantInt::get(i64Ty, static_cast<std::int64_t>(sizeOfConnectedPorts()))},
            {i64Ty},
            "rtUnionWatermarkDestroy");

        if (mOutOfOrder) {
            auto *leftTypesGlobal = buildTypeIds(
                module, context, mSchema, "union_types_" + std::to_string(getPId()));

            mUnionBufferState = stateRegistry.createHandle(
                "union_buffer_" + std::to_string(getPId()),
                "rtUnionBufferCreate",
                {
                    llvm::ConstantInt::get(i8Ty, mOutOfOrder ? 1 : 0),
                    leftTypesGlobal,
                    llvm::ConstantInt::get(i64Ty, mSchema.size())
                },
                {i8Ty, ptrTy, i64Ty},
                "rtUnionBufferDestroy"
            );
        }
    }

    llvm::GlobalVariable *UnionOperator::buildTypeIds(llvm::Module &module,
                                                      llvm::LLVMContext &ctx,
                                                      const core::logical::port::Port &schema,
                                                      const std::string &name) {
        auto *i8Ty = llvm::Type::getInt8Ty(ctx);
        std::vector<llvm::Constant *> values;
        values.reserve(schema.size());
        for (const auto &slot: schema) {
            values.push_back(llvm::ConstantInt::get(i8Ty, slot.valueTypeId));
        }
        auto *arrTy = llvm::ArrayType::get(i8Ty, values.size());
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,clang-analyzer-cplusplus.NewDeleteLeaks)
        return new llvm::GlobalVariable(module, arrTy, true, llvm::GlobalValue::PrivateLinkage, llvm::ConstantArray::get(arrTy, values), name);
    }

    void UnionOperator::consumePunctuation(core::physical::segment::SegmentContext &segmentCtx,
                                           core::shared::identifiers::PortId inputPort,
                                           const core::physical::data::PunctuationPackage &punctuation,
                                           const core::physical::segment::SegmentDataConsumers &consumers) {
        llvm::LLVMContext &ctx = segmentCtx.builder.getContext();
        llvm::Module *module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto *i64Ty = llvm::Type::getInt64Ty(ctx);
        auto *i8Ty = llvm::Type::getInt8Ty(ctx);
        auto *ptrTy = llvm::PointerType::getUnqual(ctx);

        llvm::Value *watermarkStatePtr = segmentCtx.builder.CreateLoad(ptrTy, mWatermarkState.var);

        auto* wFn = segmentCtx.builder.GetInsertBlock()->getParent();

        auto *emitBB = llvm::BasicBlock::Create(ctx, "union_emit_watermark", wFn);
        auto *mergeBB = llvm::BasicBlock::Create(ctx, "union_watermark_merge", wFn);
        auto *mergeSuppressBB = llvm::BasicBlock::Create(ctx, "heartbeat_merge", wFn);

        if (mOutOfOrder) {
            auto* checkHeartbeatBB = llvm::BasicBlock::Create(ctx, "check_heartbeat", wFn);
            auto* checkBehindBB = llvm::BasicBlock::Create(ctx, "check_behind", wFn);
            auto* suppressBB = llvm::BasicBlock::Create(ctx, "heartbeat_suppress", wFn);
            auto* continueBB = llvm::BasicBlock::Create(ctx, "heartbeat_continue", wFn);

            segmentCtx.builder.CreateBr(checkHeartbeatBB);
            segmentCtx.builder.SetInsertPoint(checkHeartbeatBB);

            llvm::Value* isHeartbeatTrue = segmentCtx.builder.CreateICmpEQ(punctuation.isHeartbeat, llvm::ConstantInt::getTrue(ctx));

            segmentCtx.builder.CreateCondBr(isHeartbeatTrue, checkBehindBB, continueBB);

            segmentCtx.builder.SetInsertPoint(checkBehindBB);

            const auto watermarkFn = module->getOrInsertFunction("rtUnionGetGlobalWatermark", llvm::FunctionType::get(i64Ty, {ptrTy}, false));
            llvm::Value* currentWatermark = segmentCtx.builder.CreateCall(
                watermarkFn, {watermarkStatePtr});

            llvm::Value* isBehind = segmentCtx.builder.CreateICmpSLT(
                punctuation.watermark, currentWatermark);

            segmentCtx.builder.CreateCondBr(isBehind, suppressBB, continueBB);

            segmentCtx.builder.SetInsertPoint(suppressBB);

            segmentCtx.builder.CreateBr(mergeSuppressBB);

            segmentCtx.builder.SetInsertPoint(continueBB);

            segmentCtx.builder.CreateBr(mergeSuppressBB);

            segmentCtx.builder.SetInsertPoint(mergeSuppressBB);
        } else {
            segmentCtx.builder.CreateBr(mergeSuppressBB);

            segmentCtx.builder.SetInsertPoint(mergeSuppressBB);
        }

        llvm::Value *isFinal = segmentCtx.builder.CreateZExt(punctuation.isFinal, i8Ty);
        llvm::Value *inputIndex = llvm::ConstantInt::get(i64Ty, static_cast<std::int64_t>(compactInputIndex(inputPort)));

        llvm::FunctionType *fnType = llvm::FunctionType::get(i8Ty, {ptrTy, i64Ty, i64Ty, i8Ty, ptrTy, ptrTy}, false);
        const llvm::FunctionCallee fn = module->getOrInsertFunction("rtUnionUpdateWatermark", fnType);

        auto *outWatermarkSlot = segmentCtx.storage.getNamed("union_outWatermarkSlot." + std::to_string(getPId()),
                                                             i64Ty);
        auto *outIsFinalSlot = segmentCtx.storage.getNamed("union_outIsFinalSlot." + std::to_string(getPId()), i8Ty);

        llvm::Value *sEmit = segmentCtx.builder.CreateCall(fn, {
                                                               watermarkStatePtr,
                                                               inputIndex,
                                                               punctuation.watermark,
                                                               isFinal,
                                                               outWatermarkSlot,
                                                               outIsFinalSlot
                                                           }, "should_emit");

        llvm::Value *condition = segmentCtx.builder.CreateICmpNE(
            sEmit, llvm::ConstantInt::get(i8Ty, 0));

        segmentCtx.builder.CreateCondBr(condition, emitBB, mergeBB);

        segmentCtx.builder.SetInsertPoint(emitBB);
        llvm::Value *newOpGlobWatermark = segmentCtx.builder.CreateLoad(i64Ty, outWatermarkSlot);
        llvm::Value *newOpGlobIsFinal = segmentCtx.builder.CreateLoad(i8Ty, outIsFinalSlot);
        llvm::Value *newIsFinal = segmentCtx.builder.CreateICmpNE(
            newOpGlobIsFinal, llvm::ConstantInt::get(i8Ty, 0));

        const auto outgoingPunctuation = core::physical::data::PunctuationPackage::create(segmentCtx.builder, newOpGlobWatermark, newIsFinal);

        consumers.front().consumePunctuation(segmentCtx, outgoingPunctuation);

        segmentCtx.builder.CreateBr(mergeBB);

        segmentCtx.builder.SetInsertPoint(mergeBB);

        if (mOutOfOrder) {
            flushBuffer(segmentCtx, consumers);
        }
    }

    void UnionOperator::flushBuffer(core::physical::segment::SegmentContext &segmentCtx, const core::physical::segment::SegmentDataConsumers &consumers) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto* i64Ty = llvm::Type::getInt64Ty(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);
        auto* voidTy = llvm::Type::getVoidTy(ctx);

        llvm::Value *watermarkStatePtr = segmentCtx.builder.CreateLoad(ptrTy, mWatermarkState.var);
        llvm::Value *bufferStatePtr = segmentCtx.builder.CreateLoad(ptrTy, mUnionBufferState.var);

        const auto lockFn = module->getOrInsertFunction("rtUnionLock", llvm::FunctionType::get(voidTy, {ptrTy}, false));
        segmentCtx.builder.CreateCall(lockFn, {bufferStatePtr});

        const auto watermarkFn = module->getOrInsertFunction("rtUnionGetGlobalWatermark",llvm::FunctionType::get(i64Ty, {ptrTy}, false));

        llvm::Value *currentWatermark = segmentCtx.builder.CreateCall(watermarkFn, {watermarkStatePtr});

        llvm::Type* batchResultTy = llvm::StructType::get(ctx, {i64Ty, ptrTy});
        const auto getReadyFn = module->getOrInsertFunction("rtUnionBufferGetReadyIndices", llvm::FunctionType::get(batchResultTy, {ptrTy, i64Ty, i64Ty}, false));
        llvm::Value* maxBatchSize = llvm::ConstantInt::get(i64Ty, 32);

        auto* fetchBB = llvm::BasicBlock::Create(ctx, "union_fetch_batch", segmentCtx.builder.GetInsertBlock()->getParent());
        auto* checkBB = llvm::BasicBlock::Create(ctx, "union_check_empty", segmentCtx.builder.GetInsertBlock()->getParent());
        auto* loopHeaderBB = llvm::BasicBlock::Create(ctx, "union_batch_loop_header", segmentCtx.builder.GetInsertBlock()->getParent());
        auto* loopBodyBB = llvm::BasicBlock::Create(ctx, "union_batch_loop_body", segmentCtx.builder.GetInsertBlock()->getParent());
        auto* removeBB = llvm::BasicBlock::Create(ctx, "union_remove_batch", segmentCtx.builder.GetInsertBlock()->getParent());
        auto* flushExitBB = llvm::BasicBlock::Create(ctx, "union_flush_exit", segmentCtx.builder.GetInsertBlock()->getParent());

        segmentCtx.builder.CreateBr(fetchBB);

        segmentCtx.builder.SetInsertPoint(fetchBB);
        llvm::Value* batchResult = segmentCtx.builder.CreateCall(getReadyFn, {bufferStatePtr, currentWatermark, maxBatchSize});
        llvm::Value* batchCount = segmentCtx.builder.CreateExtractValue(batchResult, {0});
        llvm::Value* batchIndices = segmentCtx.builder.CreateExtractValue(batchResult, {1});
        llvm::Value* isEmpty = segmentCtx.builder.CreateICmpEQ(batchCount, llvm::ConstantInt::get(i64Ty, 0));
        segmentCtx.builder.CreateCondBr(isEmpty, flushExitBB, checkBB);

        segmentCtx.builder.SetInsertPoint(checkBB);
        auto* loopIdx = segmentCtx.storage.getNamed("union_flush_loopIdx." + std::to_string(getPId()), i64Ty);
        segmentCtx.builder.CreateStore(llvm::ConstantInt::get(i64Ty, 0), loopIdx);
        segmentCtx.builder.CreateBr(loopHeaderBB);

        segmentCtx.builder.SetInsertPoint(loopHeaderBB);
        llvm::Value* currentIdx = segmentCtx.builder.CreateLoad(i64Ty, loopIdx);
        llvm::Value* isLoopDone = segmentCtx.builder.CreateICmpSGE(currentIdx, batchCount);
        segmentCtx.builder.CreateCondBr(isLoopDone, removeBB, loopBodyBB);

        segmentCtx.builder.SetInsertPoint(loopBodyBB);
        const auto getElementFn = module->getOrInsertFunction("rtUnionBufferGetBatchElement", llvm::FunctionType::get(i64Ty, {batchResultTy, i64Ty}, false));
        llvm::Value* flushIndex = segmentCtx.builder.CreateCall(getElementFn, {batchResult, currentIdx});
        emitBufferedEntry(segmentCtx, consumers, flushIndex);
        llvm::Value* nextIdx = segmentCtx.builder.CreateAdd(currentIdx, llvm::ConstantInt::get(i64Ty, 1));
        segmentCtx.builder.CreateStore(nextIdx, loopIdx);
        segmentCtx.builder.CreateBr(loopHeaderBB);

        segmentCtx.builder.SetInsertPoint(removeBB);
        const auto removeFn = module->getOrInsertFunction("rtUnionBufferRemoveV", llvm::FunctionType::get(voidTy, {ptrTy, ptrTy, i64Ty}, false));
        segmentCtx.builder.CreateCall(removeFn, {bufferStatePtr, batchIndices, batchCount});
        segmentCtx.builder.CreateBr(fetchBB);

        segmentCtx.builder.SetInsertPoint(flushExitBB);
        const auto unlockFn = module->getOrInsertFunction("rtUnionUnlock", llvm::FunctionType::get(voidTy, {ptrTy}, false));
        segmentCtx.builder.CreateCall(unlockFn, {bufferStatePtr});
    }

    void UnionOperator::emitBufferedEntry(core::physical::segment::SegmentContext &segmentCtx, const core::physical::segment::SegmentDataConsumers &consumers, llvm::Value *index) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto* i64Ty = llvm::Type::getInt64Ty(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);

        llvm::Value *bufferStatePtr = segmentCtx.builder.CreateLoad(ptrTy, mUnionBufferState.var);

        const auto getStartTsFn = module->getOrInsertFunction("rtUnionBufferGetStartTimestamp",
        llvm::FunctionType::get(i64Ty, {ptrTy, i64Ty}, false));
        const auto getEndTsFn = module->getOrInsertFunction("rtUnionBufferGetEndTimestamp",
            llvm::FunctionType::get(i64Ty, {ptrTy, i64Ty}, false));
        const auto getStartTbFn = module->getOrInsertFunction("rtUnionBufferGetStartTibreaker",
            llvm::FunctionType::get(i64Ty, {ptrTy, i64Ty}, false));
        const auto getEndTbFn = module->getOrInsertFunction("rtUnionBufferGetEndTibreaker",
            llvm::FunctionType::get(i64Ty, {ptrTy, i64Ty}, false));

        llvm::Value* startTs = segmentCtx.builder.CreateCall(getStartTsFn, {bufferStatePtr, index});
        llvm::Value* endTs = segmentCtx.builder.CreateCall(getEndTsFn, {bufferStatePtr, index});
        llvm::Value* startTb = segmentCtx.builder.CreateCall(getStartTbFn, {bufferStatePtr, index});
        llvm::Value* endTb = segmentCtx.builder.CreateCall(getEndTbFn, {bufferStatePtr, index});

        core::physical::data::DataPackage reconstructedTuple(
            core::physical::time::TimeInterval(segmentCtx.builder,
                    core::physical::time::TimePoint(startTs, startTb),
                    core::physical::time::TimePoint(endTs, endTb))
                    );
        reconstructedTuple.reserve(mSchema.size());

        const auto getFieldPtrFn = module->getOrInsertFunction("rtUnionBufferGetFieldPtr",
        llvm::FunctionType::get(ptrTy, {ptrTy, i64Ty, i64Ty}, false));

        for (std::size_t i = 0; i < mSchema.size(); ++i) {
            llvm::Value* fieldIndexVal = llvm::ConstantInt::get(i64Ty, i);
            llvm::Value* fieldPtr = segmentCtx.builder.CreateCall(
                getFieldPtrFn, {bufferStatePtr, index, fieldIndexVal});

            llvm::Type* fieldTy = segmentCtx.codegenTypeRegistry.resolveType(
                mSchema.at(i).valueTypeId, ctx);

            reconstructedTuple.pushBack(segmentCtx.builder.CreateLoad(fieldTy, fieldPtr));
        }

        consumers.front().consume(segmentCtx, reconstructedTuple);
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::union_::physical