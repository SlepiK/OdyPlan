#include <basic/operators/pipe/window/physical/count_window_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::physical {
    CountWindowOperator::CountWindowOperator(core::physical::port::Port inputPort,
                                             core::physical::port::Port outputPort,
                                             const basic::window::count::CountWindowConfig& config,
                                             core::logical::port::Port schema)
    : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)},
                    core::physical::port::Ports{std::move(outputPort)}),
          mConfig(config),
          mDataSchema(std::move(schema)) {}

    void CountWindowOperator::declareState(core::physical::state::StateRegistry& stateRegistry,
                                           llvm::Module& module,
                                           llvm::LLVMContext& context) {
        auto* i8Ty = llvm::Type::getInt8Ty(context);
        auto* i64Ty = llvm::Type::getInt64Ty(context);
        auto* ptrTy = llvm::PointerType::getUnqual(context);

        auto* typesGlobal = buildTypeIds(
            module, context, mDataSchema, "count_window_types_" + std::to_string(getPId()));

        mState = stateRegistry.createHandle("count_window_" + std::to_string(getPId()),
                                            "rtCreateCountWindowBuffer",
                                            {
                                             llvm::ConstantInt::get(i8Ty, static_cast<std::uint8_t>(mConfig.type)),
                                             llvm::ConstantInt::get(i64Ty, mConfig.size),
                                             llvm::ConstantInt::get(i64Ty, mConfig.slide),
                                             llvm::ConstantInt::get(i64Ty, mConfig.advance),
                                             typesGlobal,
                                             llvm::ConstantInt::get(i64Ty, mDataSchema.size())},
                                            {i8Ty, i64Ty, i64Ty, i64Ty, ptrTy, i64Ty},
                                            "rtDestroyCountWindowBuffer");
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
            "rtCountWindowEndRow", llvm::FunctionType::get(voidTy, {ptrTy, i64Ty}, false));

        segmentCtx.builder.CreateCall(endWindowFn, {state, tuple.timestamp});

        const auto computeWindowFn = module->getOrInsertFunction(
            "rtCountWindowCompute", llvm::FunctionType::get(voidTy, {ptrTy}, false));
        segmentCtx.builder.CreateCall(computeWindowFn, {state});

        llvm::Function* drainFn = getOrCreateDrainFn(segmentCtx, consumers.front());
        segmentCtx.builder.CreateCall(drainFn, {state});
    }

    void CountWindowOperator::end(core::physical::segment::SegmentContext& segmentCtx,
             core::shared::identifiers::PortId  /*inputPort*/,
             const core::physical::segment::SegmentDataConsumers& consumers) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto* voidTy = llvm::Type::getVoidTy(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);

        llvm::Value* state = loadStatePtr(segmentCtx);

        const auto flushFn = module->getOrInsertFunction(
            "rtCountWindowFlush", llvm::FunctionType::get(voidTy, {ptrTy}, false));
        segmentCtx.builder.CreateCall(flushFn, {state});

        llvm::Function* drainFn = getOrCreateDrainFn(segmentCtx, consumers.front());
        segmentCtx.builder.CreateCall(drainFn, {state});
    }


    void CountWindowOperator::consumePunctuation(core::physical::segment::SegmentContext &segmentCtx,
                            core::shared::identifiers::PortId  /*inputPort*/,
                            const core::physical::data::PunctuationPackage &punctuation,
                            const core::physical::segment::SegmentDataConsumers &consumers) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto* voidTy = llvm::Type::getVoidTy(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);
        auto* i8Ty = llvm::Type::getInt8Ty(ctx);

        llvm::Value* state = loadStatePtr(segmentCtx);
        llvm::Value* isFinalWatermark = segmentCtx.builder.CreateZExt(punctuation.isFinal, i8Ty);

        llvm::Value* condition = segmentCtx.builder.CreateICmpNE(isFinalWatermark, llvm::ConstantInt::get(i8Ty,0));

        llvm::Function* fn = segmentCtx.builder.GetInsertBlock()->getParent();
        auto* flushBB = llvm::BasicBlock::Create(ctx,"wm_punctuation_flush", fn);
        auto* mergeBB = llvm::BasicBlock::Create(ctx,"wm_punctuation_merge", fn);

        segmentCtx.builder.CreateCondBr(condition, flushBB, mergeBB);

        segmentCtx.builder.SetInsertPoint(flushBB);

        const auto flushFn = module->getOrInsertFunction(
            "rtCountWindowFlush", llvm::FunctionType::get(voidTy, {ptrTy}, false));
        segmentCtx.builder.CreateCall(flushFn, {state});

        llvm::Function* drainFn = getOrCreateDrainFn(segmentCtx, consumers.front());
        segmentCtx.builder.CreateCall(drainFn, {state});

        segmentCtx.builder.CreateBr(mergeBB);

        segmentCtx.builder.SetInsertPoint(mergeBB);

        for (const auto& consumer : consumers) {
            consumer.consumePunctuation(segmentCtx, punctuation);
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
            drainFn, entryBB, ctx, segmentCtx.codegenTypeRegistry);

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
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);

        const auto windowCountFn = module->getOrInsertFunction(
            "rtCountWindowGetCount", llvm::FunctionType::get(i64Ty, {ptrTy}, false));
        llvm::Value* wCount = segmentCtx.builder.CreateCall(windowCountFn, {state});

        auto* windowIndex = segmentCtx.storage.getNamed("windowIndex." +std::to_string(getPId()), i64Ty);
        segmentCtx.builder.CreateStore(llvm::ConstantInt::get(i64Ty, 0), windowIndex);

        auto* wHeaderBB = llvm::BasicBlock::Create(ctx, "windowHeader", fn);
        auto* wBodyBB = llvm::BasicBlock::Create(ctx, "windowBody", fn);
        auto* wExitBB = llvm::BasicBlock::Create(ctx, "windowExit", fn);

        segmentCtx.builder.CreateBr(wHeaderBB);
        segmentCtx.builder.SetInsertPoint(wHeaderBB);

        llvm::Value* wIndex = segmentCtx.builder.CreateLoad(i64Ty, windowIndex);
        llvm::Value* wCond = segmentCtx.builder.CreateICmpSLT(wIndex, wCount);

        segmentCtx.builder.CreateCondBr(wCond, wBodyBB, wExitBB);

        segmentCtx.builder.SetInsertPoint(wBodyBB);


        llvm::Value* wStart = nullptr;
        if (mConfig.type == basic::window::WindowType::SLIDING) {
            const auto windowGetStartFn = module->getOrInsertFunction(
           "rtCountWindowGetStart", llvm::FunctionType::get(i64Ty, {ptrTy, i64Ty}, false));
            wStart = segmentCtx.builder.CreateCall(windowGetStartFn, {state, wIndex});
        }

        llvm::Value* hasEndSlot = segmentCtx.storage.getNamed("hasEnd." +std::to_string(getPId()), llvm::Type::getInt1Ty(ctx));

        const auto windowGetEndFn = module->getOrInsertFunction(
           "rtCountWindowGetEnd", llvm::FunctionType::get(i64Ty, {ptrTy, i64Ty, ptrTy}, false));
        llvm::Value* wEnd = segmentCtx.builder.CreateCall(windowGetEndFn, {state, wIndex, hasEndSlot});

        const auto windowRowCountFn = module->getOrInsertFunction(
            "rtCountWindowGetRowCountInWindow", llvm::FunctionType::get(i64Ty, {ptrTy, i64Ty}, false));
        llvm::Value *wRowCount = segmentCtx.builder.CreateCall(windowRowCountFn, {state, wIndex});

        llvm::Value* wHasEnd = segmentCtx.builder.CreateLoad(
            llvm::Type::getInt1Ty(ctx), hasEndSlot);

        const auto windowIsClosedByFlush = module->getOrInsertFunction(
           "rtCountWindowIsClosedByFlush", llvm::FunctionType::get(llvm::Type::getInt1Ty(ctx), {ptrTy, i64Ty}, false));
        llvm::Value* wIsClosedByFlush = segmentCtx.builder.CreateCall(windowIsClosedByFlush, {state, wIndex});

        auto *windowRowIndex = segmentCtx.storage.getNamed("windowRowIndex." +std::to_string(getPId()), i64Ty);
        segmentCtx.builder.CreateStore(llvm::ConstantInt::get(i64Ty, 0), windowRowIndex);

        auto *wRowHeaderBB = llvm::BasicBlock::Create(ctx, "windowRowHeader", fn);
        auto *wRowBodyBB = llvm::BasicBlock::Create(ctx, "windowRowBody", fn);
        auto *wRowExitBB = llvm::BasicBlock::Create(ctx, "windowRowExit", fn);

        segmentCtx.builder.CreateBr(wRowHeaderBB);
        segmentCtx.builder.SetInsertPoint(wRowHeaderBB);
        llvm::Value *wRowIndex = segmentCtx.builder.CreateLoad(i64Ty, windowRowIndex);
        segmentCtx.builder.CreateCondBr(
            segmentCtx.builder.CreateICmpSLT(wRowIndex, wRowCount), wRowBodyBB, wRowExitBB);

        segmentCtx.builder.SetInsertPoint(wRowBodyBB);

        if (mConfig.type == basic::window::WindowType::ADVANCE) {
            const auto windowGetStartRowFn = module->getOrInsertFunction("rtCountWindowGetStartRow", llvm::FunctionType::get(i64Ty, {ptrTy, i64Ty, i64Ty}, false));
            wStart = segmentCtx.builder.CreateCall(windowGetStartRowFn, {state, wIndex, wRowIndex});
        }

        core::physical::data::DataPackage emit;

        emit.timestamp = codegenGetTimestamp(segmentCtx, state, wIndex, wRowIndex);

        llvm::Value *nextRowIndex = segmentCtx.builder.CreateAdd(wRowIndex, llvm::ConstantInt::get(i64Ty, 1));
        llvm::Value *wIsLastInWindow = segmentCtx.builder.CreateICmpEQ(nextRowIndex, wRowCount, "is_last_in_window");

        emit.windowTag = core::physical::data::WindowTag{
            .windowStart = wStart,
            .windowEnd = wEnd,
            .hasEnd = wHasEnd,
            .isClosed = wIsLastInWindow,
            .isClosedByFlush = wIsClosedByFlush
        };

        emit.reserve(getOutputPorts().at(0).size());
        for (std::size_t i = 0; i < getOutputPorts().at(0).size(); ++i) {
            emit.pushBack(codegenGet(segmentCtx,
                                     state,
                                     wIndex,
                                     wRowIndex,
                                     llvm::ConstantInt::get(i64Ty, i),
                                     getOutputPorts().at(0).at(i).valueTypeId));
        }

        consumer.consume(segmentCtx, emit);

        segmentCtx.builder.CreateStore(nextRowIndex, windowRowIndex);
        segmentCtx.builder.CreateBr(wRowHeaderBB);

        segmentCtx.builder.SetInsertPoint(wRowExitBB);

        llvm::Value* wNext = segmentCtx.builder.CreateAdd(wIndex, llvm::ConstantInt::get(i64Ty, 1));
        segmentCtx.builder.CreateStore(wNext, windowIndex);

        segmentCtx.builder.CreateBr(wHeaderBB);

        segmentCtx.builder.SetInsertPoint(wExitBB);
    }

    llvm::Value* CountWindowOperator::codegenGet(core::physical::segment::SegmentContext& segmentCtx,
                                                llvm::Value* state,
                                                llvm::Value* windowIndex,
                                                llvm::Value* rowIndex,
                                                llvm::Value* dataIndex,
                                                const core::shared::types::ValueTypeId typeId) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto* i64Ty = llvm::Type::getInt64Ty(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);

        const auto fn = module->getOrInsertFunction(
            "rtCountWindowGetData", llvm::FunctionType::get(ptrTy, {ptrTy, i64Ty, i64Ty, i64Ty}, false));
        llvm::Value* ret =
            segmentCtx.builder.CreateCall(fn, {state, windowIndex, rowIndex, dataIndex});

        llvm::Type* dataTy = segmentCtx.codegenTypeRegistry.resolveType(typeId, ctx);
        return segmentCtx.builder.CreateLoad(dataTy, ret);
    }

    llvm::Value* CountWindowOperator::codegenGetTimestamp(core::physical::segment::SegmentContext& segmentContext,
                                           llvm::Value* state,
                                           llvm::Value* windowIndex,
                                           llvm::Value* rowIndex) {
        llvm::LLVMContext& ctx = segmentContext.builder.getContext();
        llvm::Module* module = segmentContext.builder.GetInsertBlock()->getModule();

        auto* i64Ty = llvm::Type::getInt64Ty(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);

        const auto fn = module->getOrInsertFunction(
            "rtCountWindowGetTimestamp", llvm::FunctionType::get(i64Ty, {ptrTy, i64Ty, i64Ty}, false));
        return segmentContext.builder.CreateCall(fn, {state, windowIndex, rowIndex});
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
        auto* i64Ty = llvm::Type::getInt64Ty(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);

        auto *slot = segmentCtx.storage.getNamed("slot." +std::to_string(getPId()), typeId);
        segmentCtx.builder.CreateStore(value, slot);

        const auto fn = module->getOrInsertFunction(
            "rtCountWindowPush", llvm::FunctionType::get(voidTy, {ptrTy, i64Ty, ptrTy}, false));
        segmentCtx.builder.CreateCall(fn, {state, llvm::ConstantInt::get(i64Ty, fieldIndex), slot});
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::physical