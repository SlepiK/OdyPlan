#include <basic/operators/pipe/window/physical/time_window_operator.h>
#include <llvm/CodeGenTypes/MachineValueType.h>

#include "spdlog/fmt/bundled/format.h"

namespace sierra::odyplan::v3::basic::operators::pipe::window::physical {
    TimeWindowOperator::TimeWindowOperator(core::physical::port::Port inputPort,
                                           core::physical::port::Port outputPort,
                                           const basic::window::time::TimeWindowConfig& config,
                                           core::logical::port::Port schema)
    : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)},
                        core::physical::port::Ports{std::move(outputPort)}),
          mConfig(config), mDataSchema(std::move(schema)) {}

    void TimeWindowOperator::declareState(core::physical::state::StateRegistry &stateRegistry,
                                          llvm::Module &module,
                                          llvm::LLVMContext &context) {
        auto *i64Ty = llvm::Type::getInt64Ty(context);
        auto *ptrTy = llvm::PointerType::getUnqual(context);

        auto *typeIdsGlobal = buildTypeIds(module, context, mDataSchema,
                                           "time_window_typeids_" + std::to_string(getPId()));

        mState = stateRegistry.createHandle("time_window_" + std::to_string(getPId()),
                                            "rtCreateTimeWindowBuffer",
                                            {typeIdsGlobal, llvm::ConstantInt::get(i64Ty, mDataSchema.size())},
                                            {ptrTy, i64Ty},
                                            "rtDestroyTimeWindowBuffer");
    }

    llvm::GlobalVariable *TimeWindowOperator::buildTypeIds(llvm::Module &module, llvm::LLVMContext &ctx,
                                                           const core::logical::port::Port &schema,
                                                           const std::string &name) {
        auto *i8Ty = llvm::Type::getInt8Ty(ctx);
        std::vector<llvm::Constant *> ids;

        ids.reserve(schema.size());

        for (const auto &field: schema) {
            ids.push_back(llvm::ConstantInt::get(i8Ty, field.valueTypeId));
        }

        auto *arrTy = llvm::ArrayType::get(i8Ty, ids.size());
        auto *arrInit = llvm::ConstantArray::get(arrTy, ids);

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        return new llvm::GlobalVariable(module, arrTy, true, llvm::GlobalValue::PrivateLinkage, arrInit, name);
    }

    llvm::Value *TimeWindowOperator::loadStatePtr(core::physical::segment::SegmentContext &segmentCtx) const {
        auto *ptrTy = llvm::PointerType::getUnqual(segmentCtx.builder.getContext());
        return segmentCtx.builder.CreateLoad(ptrTy, mState.var);
    }

    void TimeWindowOperator::codegenPush(core::physical::segment::SegmentContext &segmentCtx, llvm::Value *state,
                                         llvm::Value *value, const std::size_t fieldIndex) const {
        llvm::LLVMContext &ctx = segmentCtx.builder.getContext();
        llvm::Module *module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto *i64Ty = llvm::Type::getInt64Ty(ctx);
        auto *ptrTy = llvm::PointerType::getUnqual(ctx);
        auto *voidTy = llvm::Type::getVoidTy(ctx);

        const std::string slotName = "time_window_slot." + std::to_string(getPId()) + "." + std::to_string(fieldIndex);
        llvm::Value *slot = segmentCtx.storage.getNamed(slotName, value->getType());
        segmentCtx.builder.CreateStore(value, slot);

        const auto pushFn = module->getOrInsertFunction("rtTimeWindowPush",
                                                        llvm::FunctionType::get(voidTy, {ptrTy, i64Ty, ptrTy}, false));
        segmentCtx.builder.CreateCall(pushFn, {state, llvm::ConstantInt::get(i64Ty, fieldIndex), slot});
    }

    void TimeWindowOperator::codegenEndRow(core::physical::segment::SegmentContext &segmentCtx, llvm::Value *state,
                                           const core::physical::data::DataPackage &tuple, llvm::Value *eventTs,
                                           llvm::Value *windowStart, llvm::Value *calculatedWindowEnd) const {
        llvm::LLVMContext &ctx = segmentCtx.builder.getContext();
        llvm::Module *module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto *i64Ty = llvm::Type::getInt64Ty(ctx);
        auto *ptrTy = llvm::PointerType::getUnqual(ctx);
        auto *voidTy = llvm::Type::getVoidTy(ctx);

        const auto beginFn = module->getOrInsertFunction("rtTimeWindowBeginRow",
                                                         llvm::FunctionType::get(voidTy, {ptrTy}, false));
        segmentCtx.builder.CreateCall(beginFn, {state});

        for (std::size_t i = 0; i < mDataSchema.size(); ++i) {
            codegenPush(segmentCtx, state, tuple.at(i), i);
        }

        const auto endRowFn = module->getOrInsertFunction("rtTimeWindowEndRow",
                                                          llvm::FunctionType::get(
                                                              voidTy, {ptrTy, i64Ty, i64Ty, i64Ty}, false));
        segmentCtx.builder.CreateCall(endRowFn, {state, eventTs, windowStart, calculatedWindowEnd});
    }

    void TimeWindowOperator::codegenPreTumbling(core::physical::segment::SegmentContext &segmentCtx, llvm::Value *state, const core::physical::data::DataPackage &tuple, llvm::Value *eventTs) const {
        llvm::LLVMContext &ctx = segmentCtx.builder.getContext();

        auto* i64Ty = llvm::Type::getInt64Ty(ctx);

        llvm::Value* sizeValue = llvm::ConstantInt::get(i64Ty, mConfig.size.count());

        //llvm based static pre-calculations to get the window data
        llvm::Value* k = segmentCtx.builder.CreateSDiv(eventTs, sizeValue);
        llvm::Value* windowStart = segmentCtx.builder.CreateMul(k, sizeValue);
        llvm::Value* windowEnd = segmentCtx.builder.CreateAdd(windowStart, sizeValue);

        codegenEndRow(segmentCtx, state, tuple, eventTs, windowStart, windowEnd);
    }

    void TimeWindowOperator::codegenPreIter(core::physical::segment::SegmentContext &segmentCtx, llvm::Value *state,
                                            const core::physical::data::DataPackage &tuple, llvm::Value *eventTs,
                                            const std::uint64_t calculatedWindowCandidatesPerTuple) const {
        llvm::LLVMContext &ctx = segmentCtx.builder.getContext();
        llvm::Function *fn = segmentCtx.builder.GetInsertBlock()->getParent();

        auto *i64Ty = llvm::Type::getInt64Ty(ctx);

        const std::int64_t sizeMs = mConfig.size.count();
        const std::int64_t slideMs = mConfig.slide.count();

        llvm::Value *sizeValue = llvm::ConstantInt::get(i64Ty, sizeMs);
        llvm::Value *slideValue = llvm::ConstantInt::get(i64Ty, slideMs);

        llvm::Value *kMax = segmentCtx.builder.CreateSDiv(eventTs, slideValue);
        llvm::Value *alignedStartTs = segmentCtx.builder.CreateMul(kMax, slideValue);

        auto *iSlot = segmentCtx.storage.getNamed("time_window_islot." + std::to_string(getPId()), i64Ty);
        segmentCtx.builder.CreateStore(llvm::ConstantInt::get(i64Ty, 0), iSlot);

        auto *headerBB = llvm::BasicBlock::Create(ctx, "time_window_iter_header", fn);
        auto *bodyBB = llvm::BasicBlock::Create(ctx, "time_window_iter_body", fn);
        auto *exitBB = llvm::BasicBlock::Create(ctx, "time_window_iter_exit", fn);

        segmentCtx.builder.CreateBr(headerBB);
        segmentCtx.builder.SetInsertPoint(headerBB);

        llvm::Value *iValue = segmentCtx.builder.CreateLoad(i64Ty, iSlot);
        llvm::Value *condition = segmentCtx.builder.CreateICmpSLT(
            iValue, llvm::ConstantInt::get(i64Ty, calculatedWindowCandidatesPerTuple));
        segmentCtx.builder.CreateCondBr(condition, bodyBB, exitBB);
        segmentCtx.builder.SetInsertPoint(bodyBB);

        llvm::Value *offset = segmentCtx.builder.CreateMul(iValue, slideValue);
        llvm::Value *windowStart = segmentCtx.builder.CreateSub(alignedStartTs, offset);
        llvm::Value *calculatedWindowEnd = segmentCtx.builder.CreateAdd(windowStart, sizeValue);

        llvm::Value *validWindowStart = segmentCtx.builder.CreateICmpSGE(windowStart, llvm::ConstantInt::get(i64Ty, 0));
        llvm::Value *validCalculatedWindowEnd = segmentCtx.builder.CreateICmpSLT(eventTs, calculatedWindowEnd);
        llvm::Value *isValid = segmentCtx.builder.CreateAnd(validWindowStart, validCalculatedWindowEnd);

        auto *insertBB = llvm::BasicBlock::Create(ctx, "time_window_insert", fn);
        auto *continueBB = llvm::BasicBlock::Create(ctx, "time_window_continue", fn);
        segmentCtx.builder.CreateCondBr(isValid, insertBB, continueBB);

        segmentCtx.builder.SetInsertPoint(insertBB);
        codegenEndRow(segmentCtx, state, tuple, eventTs, windowStart, calculatedWindowEnd);
        segmentCtx.builder.CreateBr(continueBB);

        segmentCtx.builder.SetInsertPoint(continueBB);
        llvm::Value *next = segmentCtx.builder.CreateAdd(iValue, llvm::ConstantInt::get(i64Ty, 1));
        segmentCtx.builder.CreateStore(next, iSlot);
        segmentCtx.builder.CreateBr(headerBB);

        segmentCtx.builder.SetInsertPoint(exitBB);
    }

    void TimeWindowOperator::consume(core::physical::segment::SegmentContext &segmentCtx,
                                     core::shared::identifiers::PortId inputPort,
                                     const core::physical::data::DataPackage &tuple,
                                     const core::physical::segment::SegmentDataConsumers &consumers) {
        llvm::LLVMContext &ctx = segmentCtx.builder.getContext();
        llvm::Module *module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto *i64Ty = llvm::Type::getInt64Ty(ctx);

        llvm::Value *state = loadStatePtr(segmentCtx);

        //Guard for EventTimestamp for negative values
        llvm::Function *smaxFn = llvm::Intrinsic::getOrInsertDeclaration(module, llvm::Intrinsic::smax, {i64Ty});
        llvm::Value *eventTs = segmentCtx.builder.CreateCall(
            smaxFn, {tuple.timestamp, llvm::ConstantInt::get(i64Ty, 0)});

        const std::int64_t sizeMs = mConfig.size.count();
        const std::int64_t slideMs = mConfig.slide.count();
        const std::int64_t calculatedWindowsPerTuple = (sizeMs + slideMs - 1) / slideMs;
        assert(
            calculatedWindowsPerTuple >= 0 &&
            "TimeWindowOperator::consume: calculatedWindowsPerTuple must be postive - check logical::TimeWindowOperator::validateInput()");

        if (calculatedWindowsPerTuple == 1) {
            codegenPreTumbling(segmentCtx, state, tuple, eventTs);
        } else {
            codegenPreIter(segmentCtx, state, tuple, eventTs, calculatedWindowsPerTuple);
        }
    }

    llvm::Value *TimeWindowOperator::codegenGet(core::physical::segment::SegmentContext &segmentCtx, llvm::Value *state,
                                                llvm::Value *rowIndex, const std::size_t fieldIndex,
                                                const core::shared::types::ValueTypeId typeId) {
        llvm::LLVMContext &ctx = segmentCtx.builder.getContext();
        llvm::Module *module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto *i64Ty = llvm::Type::getInt64Ty(ctx);
        auto *ptrTy = llvm::PointerType::getUnqual(ctx);

        const auto getFn = module->getOrInsertFunction("rtTimeWindowGetDrainedData", llvm::FunctionType::get(ptrTy, {ptrTy, i64Ty, i64Ty}, false));
        llvm::Value* rawPtr = segmentCtx.builder.CreateCall(getFn, {state, rowIndex, llvm::ConstantInt::get(i64Ty, fieldIndex)});

        llvm::Type* fieldTy = segmentCtx.codegenTypeRegistry.resolveType(typeId, ctx);

        return segmentCtx.builder.CreateLoad(fieldTy, rawPtr);
    }

    llvm::Function *TimeWindowOperator::getOrCreateDrainFn(core::physical::segment::SegmentContext &segmentCtx,
                                                           const core::physical::segment::SegmentConsumerHandle &
                                                           consumer) {
        if (mDrainFn != nullptr) {
            return mDrainFn;
        }

        llvm::LLVMContext &ctx = segmentCtx.builder.getContext();
        llvm::Module *module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto *i64Ty = llvm::Type::getInt64Ty(ctx);
        auto *i8Ty = llvm::Type::getInt8Ty(ctx);
        auto *ptrTy = llvm::PointerType::getUnqual(ctx);
        auto *voidTy = llvm::Type::getVoidTy(ctx);

        auto *fnType = llvm::FunctionType::get(voidTy, {ptrTy, i64Ty, i8Ty}, false);
        mDrainFn = llvm::Function::Create(fnType, llvm::Function::PrivateLinkage,
                                          "time_window_drain_" + std::to_string(getPId()), module);

        llvm::Value *state = mDrainFn->getArg(0);
        llvm::Value *threshold = mDrainFn->getArg(1);
        llvm::Value *isFinal = mDrainFn->getArg(2);

        auto *entryBB = llvm::BasicBlock::Create(ctx, "entry", mDrainFn);
        core::physical::segment::SegmentContext drainCtx(mDrainFn, entryBB, ctx, segmentCtx.codegenTypeRegistry);

        const auto drainFn = module->getOrInsertFunction("rtTimeWindowDrain",
                                                         llvm::FunctionType::get(voidTy, {ptrTy, i64Ty, i8Ty}, false));
        drainCtx.builder.CreateCall(drainFn, {state, threshold, isFinal});

        const auto countFn = module->getOrInsertFunction("rtTimeWindowGetDrainedRowCount",
                                                         llvm::FunctionType::get(i64Ty, {ptrTy}, false));
        llvm::Value *count = drainCtx.builder.CreateCall(countFn, {state});

        auto *iSlot = drainCtx.storage.getNamed("time_window_drain_islot." + std::to_string(getPId()), i64Ty);
        drainCtx.builder.CreateStore(llvm::ConstantInt::get(i64Ty, 0), iSlot);

        auto *headerBB = llvm::BasicBlock::Create(ctx, "drainHeader", mDrainFn);
        auto *bodyBB = llvm::BasicBlock::Create(ctx, "drainBody", mDrainFn);
        auto *exitBB = llvm::BasicBlock::Create(ctx, "drainExit", mDrainFn);

        drainCtx.builder.CreateBr(headerBB);
        drainCtx.builder.SetInsertPoint(headerBB);
        llvm::Value *iValue = drainCtx.builder.CreateLoad(i64Ty, iSlot);

        llvm::Value *condition = drainCtx.builder.CreateICmpULT(iValue, count);
        drainCtx.builder.CreateCondBr(condition, bodyBB, exitBB);

        drainCtx.builder.SetInsertPoint(bodyBB);

        const auto getWindowStart = module->getOrInsertFunction("rtTimeWindowGetDrainedWindowStart",
                                                                llvm::FunctionType::get(i64Ty, {ptrTy, i64Ty}, false));
        const auto getWindowEnd = module->getOrInsertFunction("rtTimeWindowGetDrainedWindowEnd",
                                                              llvm::FunctionType::get(i64Ty, {ptrTy, i64Ty}, false));
        const auto getTs = module->getOrInsertFunction("rtTimeWindowGetDrainedTimestamp",
                                                       llvm::FunctionType::get(i64Ty, {ptrTy, i64Ty}, false));
        const auto getIsLast = module->getOrInsertFunction("rtTimeWindowGetDrainedIsLastInWindow",
                                                           llvm::FunctionType::get(i8Ty, {ptrTy, i64Ty}, false));


        llvm::Value *windowStart = drainCtx.builder.CreateCall(getWindowStart, {state, iValue});
        llvm::Value *windowEnd = drainCtx.builder.CreateCall(getWindowEnd, {state, iValue});
        llvm::Value *timestamp = drainCtx.builder.CreateCall(getTs, {state, iValue});
        llvm::Value* isLast = drainCtx.builder.CreateCall(getIsLast, {state, iValue});
        llvm::Value* isLastInWindow = drainCtx.builder.CreateICmpNE(isLast, llvm::ConstantInt::get(i8Ty, 0));

        core::physical::data::DataPackage dTuple;
        dTuple.timestamp = timestamp;
        dTuple.windowTag = core::physical::data::WindowTag{
            .windowStart = windowStart,
            .windowEnd = windowEnd,
            //time_window knows its >calculated< end
            .hasEnd = llvm::ConstantInt::getTrue(ctx),
            //window should be marked as closed, if this is the last drained entry/element (punctuation and allowed lateness should help guarding this a little...)
            .isClosed = isLastInWindow
        };
        dTuple.reserve(mDataSchema.size());

        for (std::size_t i = 0; i < mDataSchema.size(); ++i) {
            dTuple.pushBack(codegenGet(drainCtx, state, iValue, i, mDataSchema.at(i).valueTypeId));
        }

        consumer.consume(drainCtx, dTuple);

        llvm::Value *next = drainCtx.builder.CreateAdd(iValue, llvm::ConstantInt::get(i64Ty, 1));
        drainCtx.builder.CreateStore(next, iSlot);
        drainCtx.builder.CreateBr(headerBB);

        drainCtx.builder.SetInsertPoint(exitBB);
        drainCtx.builder.CreateRetVoid();

        return mDrainFn;
    }

    void TimeWindowOperator::drainWindows(core::physical::segment::SegmentContext &segmentCtx, llvm::Value *state,
                                          llvm::Value *threshold, llvm::Value *isFinal,
                                          const core::physical::segment::SegmentConsumerHandle &consumer) {
        llvm::Function *fn = getOrCreateDrainFn(segmentCtx, consumer);
        segmentCtx.builder.CreateCall(fn, {state, threshold, isFinal});
    }

    void TimeWindowOperator::consumePunctuation(core::physical::segment::SegmentContext &segmentCtx,
                                                core::shared::identifiers::PortId  /*inputPort*/,
                                                const core::physical::data::PunctuationPackage &punctuation,
                                                const core::physical::segment::SegmentDataConsumers &consumers) {
        llvm::LLVMContext &ctx = segmentCtx.builder.getContext();

        auto* i64Ty = llvm::Type::getInt64Ty(ctx);
        auto* i8Ty = llvm::Type::getInt8Ty(ctx);

        llvm::Value* state = loadStatePtr(segmentCtx);

        llvm::Value* allowedLateness = llvm::ConstantInt::get(i64Ty, mConfig.allowedLateness.count());
        llvm::Value* threshold = segmentCtx.builder.CreateSub(punctuation.watermark, allowedLateness);
        llvm::Value* isFinal = segmentCtx.builder.CreateZExt(punctuation.isFinal, i8Ty);

        drainWindows(segmentCtx, state, threshold, isFinal, consumers.front());

        for (const auto& consumer : consumers) {
            consumer.consumePunctuation(segmentCtx, punctuation);
        }
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::physical