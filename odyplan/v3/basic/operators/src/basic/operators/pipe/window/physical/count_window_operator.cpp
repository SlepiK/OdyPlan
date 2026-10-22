#include <basic/operators/pipe/window/physical/count_window_operator.h>

#include "basic/operators/pipe/window/runtime/count_window_buffer.h"

namespace sierra::odyplan::v3::basic::operators::pipe::window::physical {
    CountWindowOperator::CountWindowOperator(core::physical::port::Port inputPort,
                                             core::physical::port::Port outputPort,
                                             const details::CountWindowOptions &config,
                                             core::logical::port::Port schema, const bool outOfOrder)
        : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)},
                            core::physical::port::Ports{std::move(outputPort)}),
          mOutOfOrder(outOfOrder),
          mConfig(config),
          mDataSchema(std::move(schema)) {
        assert(mConfig.advance > 0 && "Advance 0 not supported");
        assert(mConfig.size >= mConfig.advance && "Size < Advance not implemented!");
    }

    void CountWindowOperator::declareState(core::physical::state::StateRegistry& stateRegistry,
                                           llvm::Module& module,
                                           llvm::LLVMContext& context) {
        auto* ptrTy = llvm::PointerType::getUnqual(context);
        auto* sizeTTy = module.getDataLayout().getIntPtrType(context);

        auto* typesGlobal = buildTypeIds(
            module, context, mDataSchema, "count_window_types_" + std::to_string(getPId()));

        mState = stateRegistry.createHandle("count_window_" + std::to_string(getPId()),
                                            "rtCreateCountWindowBuffer",
                                            {
                                             llvm::ConstantInt::get(sizeTTy, mConfig.size),
                                             llvm::ConstantInt::get(sizeTTy, mConfig.slide),
                                             llvm::ConstantInt::get(sizeTTy, mConfig.advance),
                                             typesGlobal,
                                             llvm::ConstantInt::get(sizeTTy, mDataSchema.size())},
                                            {sizeTTy, sizeTTy, sizeTTy, ptrTy, sizeTTy},
                                            "rtDestroyCountWindowBuffer");

        if (mOutOfOrder) {
            mPunctuationState = stateRegistry.createHandle(
                "count_window_punc_" + std::to_string(getPId()),
                "rtCreatePunctuationBuffer",
                {},
                {},
                "rtDestroyPunctuationBuffer");
        }
    }


    void CountWindowOperator::consume(core::physical::segment::SegmentContext& segmentCtx,
                 core::shared::identifiers::PortId  /*inputPort*/,
                 const core::physical::data::DataPackage& tuple,
                 const core::physical::segment::SegmentDataConsumers& consumers) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto* voidTy = llvm::Type::getVoidTy(ctx);
        auto* i64Ty = llvm::Type::getInt64Ty(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);

        llvm::Value* state = loadStatePtr(segmentCtx);

        const auto beginWindowFn = module->getOrInsertFunction(
            "rtCountWindowBeginRow", llvm::FunctionType::get(voidTy, {ptrTy}, false));
        segmentCtx.builder.CreateCall(beginWindowFn, {state});

        for (std::size_t i = 0; i < getOutputPorts().at(0).size(); ++i) {
            codegenPush(
                segmentCtx, state, tuple.at(i), getOutputPorts().at(0).at(i).valueTypeId, i);
        }

        const auto endWindowFn = module->getOrInsertFunction(
            "rtCountWindowEndRow", llvm::FunctionType::get(voidTy, {ptrTy, i64Ty, i64Ty}, false));

        segmentCtx.builder.CreateCall(endWindowFn, {state, tuple.timeInterval.getStart().getPoint(), tuple.timeInterval.getEnd().getPoint()});

        llvm::Function* drainFn = getOrCreateDrainFn(segmentCtx, consumers.front());
        segmentCtx.builder.CreateCall(drainFn, {state});
    }


    void CountWindowOperator::consumePunctuation(core::physical::segment::SegmentContext &segmentCtx, core::shared::identifiers::PortId,
                            const core::physical::data::PunctuationPackage &punctuation,
                            const core::physical::segment::SegmentDataConsumers &consumers) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto* voidTy = llvm::Type::getVoidTy(ctx);
        auto* i64Ty = llvm::Type::getInt64Ty(ctx);
        auto* i1Ty = llvm::Type::getInt1Ty(ctx);
        auto* i8Ty = llvm::Type::getInt8Ty(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);
        auto* sizeTTy = module->getDataLayout().getIntPtrType(ctx);

        llvm::Value* state = loadStatePtr(segmentCtx);
        llvm::Value* puncTs = punctuation.watermark;

        if (!mOutOfOrder) {
            consumers.front().consumePunctuation(segmentCtx, punctuation);
        } else {
            const auto getMinTsFn = module->getOrInsertFunction("rtCountWindowGetMinTs",
                                                                llvm::FunctionType::get(i64Ty, {ptrTy}, false));

            llvm::Value *minTs = segmentCtx.builder.CreateCall(getMinTsFn, {state});
            llvm::Value *isSafe = segmentCtx.builder.CreateICmpSLT(puncTs, minTs);

            auto *sendBB = llvm::BasicBlock::Create(ctx, "puncSend", segmentCtx.builder.GetInsertBlock()->getParent());
            auto *bufferBB = llvm::BasicBlock::Create(ctx, "puncBuffer",
                                                      segmentCtx.builder.GetInsertBlock()->getParent());
            auto *mergeBB =
                    llvm::BasicBlock::Create(ctx, "puncMerge", segmentCtx.builder.GetInsertBlock()->getParent());

            segmentCtx.builder.CreateCondBr(isSafe, sendBB, bufferBB);

            segmentCtx.builder.SetInsertPoint(sendBB);

            consumers.front().consumePunctuation(segmentCtx, punctuation);

            segmentCtx.builder.CreateBr(mergeBB);

            segmentCtx.builder.SetInsertPoint(bufferBB);

            llvm::Value *puncState = segmentCtx.builder.CreateLoad(ptrTy, mPunctuationState.var);

            const auto storeFn = module->getOrInsertFunction(
                "rtPunctuationBufferStore",
                llvm::FunctionType::get(voidTy, {ptrTy, i64Ty, i1Ty, i1Ty}, false));

            segmentCtx.builder.CreateCall(storeFn, {
                                              puncState,
                                              punctuation.watermark,
                                              punctuation.isFinal,
                                              punctuation.isHeartbeat
                                          });

            segmentCtx.builder.CreateBr(mergeBB);

            segmentCtx.builder.SetInsertPoint(mergeBB);
        }
    }

    llvm::Function*
    CountWindowOperator::getOrCreateDrainFn(const core::physical::segment::SegmentContext& segmentCtx,
                       const core::physical::segment::SegmentConsumerHandle& consumer) {
        if (mDrainFn != nullptr) {
            return mDrainFn;
        }

        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto* voidTy = llvm::Type::getVoidTy(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);

        auto* drainFn = llvm::Function::Create(
            llvm::FunctionType::get(voidTy, {ptrTy}, false),
            llvm::Function::InternalLinkage,
            "ody_window_drain_" + std::to_string(getPId()),
            module);

        drainFn->getArg(0)->setName("windowHandle");

        auto* entryBB = llvm::BasicBlock::Create(ctx, "drainEntry", drainFn);

        core::physical::segment::SegmentContext drainCtx(
            drainFn, entryBB, ctx, *segmentCtx.codegenTypeRegistry);

        llvm::Value* state = drainFn->getArg(0);
        drainWindows(drainCtx, state, consumer);

        drainCtx.builder.CreateRetVoid();

        mDrainFn = drainFn;

        return drainFn;
    }

    void CountWindowOperator::drainWindows(core::physical::segment::SegmentContext& segmentCtx,
                      llvm::Value* state,
                      const core::physical::segment::SegmentConsumerHandle& consumer) const {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Function* fn = segmentCtx.builder.GetInsertBlock()->getParent();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto* i64Ty = llvm::Type::getInt64Ty(ctx);
        auto* i8Ty = llvm::Type::getInt8Ty(ctx);
        auto* i1Ty = llvm::Type::getInt1Ty(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);
        auto* voidTy = llvm::Type::getVoidTy(ctx);
        auto* sizeTTy = module->getDataLayout().getIntPtrType(ctx);

        const auto readyFn = module->getOrInsertFunction("rtCountWindowReady", llvm::FunctionType::get(i1Ty, {ptrTy}, false));
        const auto endFn = module->getOrInsertFunction("rtCountWindowEnd", llvm::FunctionType::get(i64Ty, {ptrTy}, false));
        const auto rowStartFn = module->getOrInsertFunction("rtCountWindowRowStart", llvm::FunctionType::get(i64Ty, {ptrTy, sizeTTy}, false));
        const auto advanceFn = module->getOrInsertFunction("rtCountWindowAdvance", llvm::FunctionType::get(voidTy, {ptrTy}, false));
        const auto emitCountFn = module->getOrInsertFunction("rtCountWindowEmitCount", llvm::FunctionType::get(sizeTTy, {ptrTy}, false));

        auto* headerBB = llvm::BasicBlock::Create(ctx, "windowHeader", fn);
        auto* bodyBB = llvm::BasicBlock::Create(ctx, "windowBody", fn);
        auto* exitBB = llvm::BasicBlock::Create(ctx, "windowExit", fn);

        segmentCtx.builder.CreateBr(headerBB);
        segmentCtx.builder.SetInsertPoint(headerBB);
        llvm::Value* isReady = segmentCtx.builder.CreateCall(readyFn, {state});
        segmentCtx.builder.CreateCondBr(isReady, bodyBB, exitBB);

        segmentCtx.builder.SetInsertPoint(bodyBB);
        llvm::Value* emitCount = segmentCtx.builder.CreateCall(emitCountFn, {state});
        llvm::Value* endTs = segmentCtx.builder.CreateCall(endFn, {state});

        const core::physical::time::TimePoint sharedEndTP(segmentCtx.builder, endTs);

        auto* rowIndexSlot = segmentCtx.storage.getNamed(std::to_string(getPId()) + ".countWindowRowIndex", sizeTTy);
        segmentCtx.builder.CreateStore(llvm::ConstantInt::get(i64Ty, 0), rowIndexSlot);

        auto* rowHeaderBB = llvm::BasicBlock::Create(ctx, "windowRowHeader", fn);
        auto* rowBodyBB = llvm::BasicBlock::Create(ctx, "windowRowBody", fn);
        auto* rowExitBB = llvm::BasicBlock::Create(ctx, "windowRowExit", fn);

        segmentCtx.builder.CreateBr(rowHeaderBB);
        segmentCtx.builder.SetInsertPoint(rowHeaderBB);
        llvm::Value* rowIndex = segmentCtx.builder.CreateLoad(sizeTTy, rowIndexSlot);
        segmentCtx.builder.CreateCondBr(segmentCtx.builder.CreateICmpSLT(rowIndex, emitCount), rowBodyBB, rowExitBB);

        segmentCtx.builder.SetInsertPoint(rowBodyBB);

        llvm::Value* rs = segmentCtx.builder.CreateCall(rowStartFn, {state, rowIndex});
        const core::physical::time::TimePoint rowStartTP = core::physical::time::TimePoint(segmentCtx.builder, rs);


        core::physical::data::DataPackage emit(core::physical::time::TimeInterval(segmentCtx.builder, rowStartTP, sharedEndTP));
        emit.reserve(getOutputPorts().at(0).size());
        for (std::size_t i = 0; i < getOutputPorts().at(0).size(); ++i) {
            emit.pushBack(codegenGet(segmentCtx, state, rowIndex,
                                     llvm::ConstantInt::get(sizeTTy, i),
                                     getOutputPorts().at(0).at(i).valueTypeId));
        }

        consumer.consume(segmentCtx, emit);

        llvm::Value* nextRow = segmentCtx.builder.CreateAdd(rowIndex, llvm::ConstantInt::get(sizeTTy, 1));
        segmentCtx.builder.CreateStore(nextRow, rowIndexSlot);
        segmentCtx.builder.CreateBr(rowHeaderBB);

        segmentCtx.builder.SetInsertPoint(rowExitBB);
        segmentCtx.builder.CreateCall(advanceFn, {state});

        if (mOutOfOrder) {
            const auto getMinTsFn = module->getOrInsertFunction(
            "rtCountWindowGetMinTs",
            llvm::FunctionType::get(i64Ty, {ptrTy}, false));
            llvm::Value* newMinTs = segmentCtx.builder.CreateCall(getMinTsFn, {state});

            const auto flushFn = module->getOrInsertFunction(
            "rtPunctuationBufferFlush",
            llvm::FunctionType::get(sizeTTy, {ptrTy, i64Ty}, false));

            llvm::Value* puncState = segmentCtx.builder.CreateLoad(ptrTy, mPunctuationState.var);
            llvm::Value* sendCount = segmentCtx.builder.CreateCall(flushFn, {puncState, newMinTs});

            auto* flushHeader = llvm::BasicBlock::Create(ctx, "flushHeader", fn);
            auto* flushBody = llvm::BasicBlock::Create(ctx, "flushBody", fn);
            auto* flushExit = llvm::BasicBlock::Create(ctx, "flushExit", fn);

            auto* iSlot = segmentCtx.storage.getNamed(
                "flushIdx_" + std::to_string(getPId()),
                sizeTTy);
            segmentCtx.builder.CreateStore(llvm::ConstantInt::get(sizeTTy, 0), iSlot);

            segmentCtx.builder.CreateBr(flushHeader);
            segmentCtx.builder.SetInsertPoint(flushHeader);
            llvm::Value* i = segmentCtx.builder.CreateLoad(sizeTTy, iSlot);
            segmentCtx.builder.CreateCondBr(
                segmentCtx.builder.CreateICmpULT(i, sendCount),
                flushBody,
                flushExit);

            segmentCtx.builder.SetInsertPoint(flushBody);

            const auto getFn = module->getOrInsertFunction(
                            "rtPunctuationBufferGetToSend",
                            llvm::FunctionType::get(ptrTy, {ptrTy, sizeTTy}, false));

            llvm::Value* puncPtr = segmentCtx.builder.CreateCall(getFn, {puncState, i});

            llvm::Value* watermark = segmentCtx.builder.CreateLoad(i64Ty,
        segmentCtx.builder.CreateBitCast(
            segmentCtx.builder.CreateGEP(
                llvm::ArrayType::get(i8Ty, 16),
                puncPtr,
                {llvm::ConstantInt::get(sizeTTy, 0), llvm::ConstantInt::get(sizeTTy, 0)},
                "",
                llvm::GEPNoWrapFlags::none()),
            llvm::PointerType::get(i64Ty, 0)));

            // isFinal (bool, offset 8)
            llvm::Value* isFinal = segmentCtx.builder.CreateTrunc(
                segmentCtx.builder.CreateLoad(i8Ty,
                    segmentCtx.builder.CreateBitCast(
                        segmentCtx.builder.CreateGEP(
                            llvm::ArrayType::get(i8Ty, 16),
                            puncPtr,
                            {llvm::ConstantInt::get(sizeTTy, 0), llvm::ConstantInt::get(sizeTTy, 8)},
                            "",
                            llvm::GEPNoWrapFlags::none()),
                        llvm::PointerType::get(i8Ty, 0))),
                i1Ty);

            // isHeartbeat (bool, offset 9)
            llvm::Value* isHeartbeat = segmentCtx.builder.CreateTrunc(
                segmentCtx.builder.CreateLoad(i8Ty,
                    segmentCtx.builder.CreateBitCast(
                        segmentCtx.builder.CreateGEP(
                            llvm::ArrayType::get(i8Ty, 16),
                            puncPtr,
                            {llvm::ConstantInt::get(sizeTTy, 0), llvm::ConstantInt::get(sizeTTy, 9)},
                            "",
                            llvm::GEPNoWrapFlags::none()),
                        llvm::PointerType::get(i8Ty, 0))),
                i1Ty);


            core::physical::data::PunctuationPackage reconPunc;
            reconPunc.watermark = watermark;
            reconPunc.isFinal = isFinal;
            reconPunc.isHeartbeat = isHeartbeat;

            consumer.consumePunctuation(segmentCtx, reconPunc);

            llvm::Value* nextI = segmentCtx.builder.CreateAdd(i, llvm::ConstantInt::get(sizeTTy, 1));
            segmentCtx.builder.CreateStore(nextI, iSlot);
            segmentCtx.builder.CreateBr(flushHeader);

            segmentCtx.builder.SetInsertPoint(flushExit);

            const auto cleanupFn = module->getOrInsertFunction(
            "rtPunctuationBufferCleanup",
            llvm::FunctionType::get(voidTy, {ptrTy}, false));
            segmentCtx.builder.CreateCall(cleanupFn, {puncState});
        }


        segmentCtx.builder.CreateBr(headerBB);

        segmentCtx.builder.SetInsertPoint(exitBB);
    }

    llvm::Value* CountWindowOperator::codegenGet(core::physical::segment::SegmentContext& segmentCtx,
                                                llvm::Value* state,
                                                llvm::Value* rowIndex,
                                                llvm::Value* dataIndex,
                                                const core::shared::types::ValueTypeId typeId) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto* ptrTy = llvm::PointerType::getUnqual(ctx);
        auto* sizeTTy = module->getDataLayout().getIntPtrType(ctx);

        const auto fn = module->getOrInsertFunction(
            "rtCountWindowGetData", llvm::FunctionType::get(ptrTy, {ptrTy, sizeTTy, sizeTTy}, false));
        llvm::Value* ret =
            segmentCtx.builder.CreateCall(fn, {state, rowIndex, dataIndex});

        llvm::Type* dataTy = segmentCtx.codegenTypeRegistry->resolveType(typeId, ctx);
        return segmentCtx.builder.CreateLoad(dataTy, ret);
    }

    llvm::Value* CountWindowOperator::loadStatePtr(core::physical::segment::SegmentContext& segmentCtx) const {
        auto* ptrTy = llvm::PointerType::getUnqual(segmentCtx.builder.getContext());
        return segmentCtx.builder.CreateLoad(ptrTy, mState.var);
    }

    llvm::GlobalVariable* CountWindowOperator::buildTypeIds(llvm::Module& module,
                                                           llvm::LLVMContext& ctx,
                                                           const core::logical::port::Port& schema,
                                                           const std::string& name) {
        auto* i8Ty = llvm::Type::getInt8Ty(ctx);

        std::vector<llvm::Constant*> values;
        values.reserve(schema.size());

        for (const auto& field : schema) {
            values.push_back(llvm::ConstantInt::get(i8Ty, field.valueTypeId));
        }

        auto* arrTy = llvm::ArrayType::get(i8Ty, values.size());

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,clang-analyzer-cplusplus.NewDeleteLeaks)
        return new llvm::GlobalVariable(module,
                                        arrTy,
                                        true,
                                        llvm::GlobalValue::InternalLinkage,
                                        llvm::ConstantArray::get(arrTy, values),
                                        name);
    }

    void CountWindowOperator::codegenPush(core::physical::segment::SegmentContext& segmentCtx,
                                         llvm::Value* state,
                                         llvm::Value* value,
                                         const core::shared::types::ValueTypeId typeId,
                                         const std::size_t fieldIndex) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto* voidTy = llvm::Type::getVoidTy(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);
        auto* sizeTTy = module->getDataLayout().getIntPtrType(ctx);

        auto *slot = segmentCtx.storage.getNamed(std::to_string(getPId()) + ".slot", typeId);
        segmentCtx.builder.CreateStore(value, slot);

        const auto fn = module->getOrInsertFunction(
            "rtCountWindowPush", llvm::FunctionType::get(voidTy, {ptrTy, sizeTTy, ptrTy}, false));
        segmentCtx.builder.CreateCall(fn, {state, llvm::ConstantInt::get(sizeTTy, fieldIndex), slot});
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::physical