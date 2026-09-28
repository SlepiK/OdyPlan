#include <basic/operators/pipe/window/physical/count_window_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::physical {
    CountWindowOperator::CountWindowOperator(core::physical::port::Port inputPort,
                                             core::physical::port::Port outputPort,
                                             const details::CountWindowOptions& config,
                                             core::logical::port::Port schema)
    : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)},
                    core::physical::port::Ports{std::move(outputPort)}),
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

        auto* rowIndexSlot = segmentCtx.storage.getNamed("countWindowRowIndex." + std::to_string(getPId()), sizeTTy);
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

        auto *slot = segmentCtx.storage.getNamed("slot." +std::to_string(getPId()), typeId);
        segmentCtx.builder.CreateStore(value, slot);

        const auto fn = module->getOrInsertFunction(
            "rtCountWindowPush", llvm::FunctionType::get(voidTy, {ptrTy, sizeTTy, ptrTy}, false));
        segmentCtx.builder.CreateCall(fn, {state, llvm::ConstantInt::get(sizeTTy, fieldIndex), slot});
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::physical