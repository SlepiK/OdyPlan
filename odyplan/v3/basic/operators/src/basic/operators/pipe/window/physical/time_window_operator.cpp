#include <basic/operators/pipe/window/physical/time_window_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::physical {
    TimeWindowOperator::TimeWindowOperator(core::physical::port::Port inputPort,
                                           core::physical::port::Port outputPort,
                                           const basic::window::time::TimeWindowConfig& config)
    : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)},
                        core::physical::port::Ports{std::move(outputPort)}),
          mConfig(config) {}

    void TimeWindowOperator::declareState(core::physical::state::StateRegistry& /*stateRegistry*/,
                                          llvm::Module& /*module*/,
                                          llvm::LLVMContext& /*context*/) {
        //No state, as its metadata based (see ::consume)
    }

    void TimeWindowOperator::emitWindow(core::physical::segment::SegmentContext& segmentCtx,
        const core::physical::data::DataPackage& tuple,
        llvm::Value* windowStart,
        llvm::Value* windowEnd,
        const core::physical::segment::SegmentDataConsumers& consumers) {
        core::physical::data::DataPackage annotated = tuple;

        annotated.windowMetadata = core::physical::data::WindowMetadata{
            .windowStart = windowStart,
            .windowEnd   = windowEnd,
            .hasKnownEnd = llvm::ConstantInt::getTrue(segmentCtx.builder.getContext())
        };

        consumers.front()(segmentCtx, annotated);
    }

    void TimeWindowOperator::codegenUnrolled(core::physical::segment::SegmentContext& segmentCtx,
            const core::physical::data::DataPackage& tuple,
            const core::physical::segment::SegmentDataConsumers& consumers,
            const std::uint64_t windowsPerTuple) const {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        auto* i64Ty = llvm::Type::getInt64Ty(ctx);
        llvm::Function* fn = segmentCtx.builder.GetInsertBlock()->getParent();

        const std::uint64_t sizeMs  = mConfig.size.count();
        const std::uint64_t slideMs = mConfig.slide.count();

        llvm::Value* ts = tuple.timestamp;
        llvm::Value* slideVal = llvm::ConstantInt::get(i64Ty, slideMs);
        llvm::Value* sizeVal  = llvm::ConstantInt::get(i64Ty, sizeMs);

        llvm::Value* kMax = segmentCtx.builder.CreateSDiv(ts, slideVal);
        llvm::Value* alignedStart = segmentCtx.builder.CreateMul(kMax, slideVal);

        for (std::uint64_t i = 0; i < windowsPerTuple; ++i) {
            llvm::Value* offset = llvm::ConstantInt::get(i64Ty, i * slideMs);
            llvm::Value* windowStart = segmentCtx.builder.CreateSub(alignedStart, offset);
            llvm::Value* windowEnd   = segmentCtx.builder.CreateAdd(windowStart, sizeVal);

            llvm::Value* validStart = segmentCtx.builder.CreateICmpSGE(
                windowStart, llvm::ConstantInt::get(i64Ty, 0));
            llvm::Value* validEnd = segmentCtx.builder.CreateICmpSLT(ts, windowEnd);
            llvm::Value* valid = segmentCtx.builder.CreateAnd(validStart, validEnd);

            auto* emitBB = llvm::BasicBlock::Create(ctx, "twEmit", fn);
            auto* contBB = llvm::BasicBlock::Create(ctx, "twCont", fn);
            segmentCtx.builder.CreateCondBr(valid, emitBB, contBB);

            segmentCtx.builder.SetInsertPoint(emitBB);
            emitWindow(segmentCtx, tuple, windowStart, windowEnd, consumers);
            segmentCtx.builder.CreateBr(contBB);

            segmentCtx.builder.SetInsertPoint(contBB);
        }
    }

    void TimeWindowOperator::codegenLoop(core::physical::segment::SegmentContext& segmentCtx,
        const core::physical::data::DataPackage& tuple,
        const core::physical::segment::SegmentDataConsumers& consumers,
        const std::uint64_t windowsPerTuple) const {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Function* fn = segmentCtx.builder.GetInsertBlock()->getParent();
        auto* i64Ty = llvm::Type::getInt64Ty(ctx);

        const std::int64_t sizeMs  = mConfig.size.count();
        const std::int64_t slideMs = mConfig.slide.count();

        llvm::Value* ts = tuple.timestamp;
        llvm::Value* slideVal = llvm::ConstantInt::get(i64Ty, slideMs);
        llvm::Value* sizeVal  = llvm::ConstantInt::get(i64Ty, sizeMs);

        llvm::Value* kMax = segmentCtx.builder.CreateSDiv(ts, slideVal);
        llvm::Value* alignedStart = segmentCtx.builder.CreateMul(kMax, slideVal);

        //Counter for windows per tuple / datapackage
        auto *iSlot = segmentCtx.storage.getNamed("iSlot." +std::to_string(getPId()), i64Ty);
        segmentCtx.builder.CreateStore(llvm::ConstantInt::get(i64Ty, 0), iSlot);

        auto* headerBB = llvm::BasicBlock::Create(ctx, "twLoopHeader", fn);
        auto* bodyBB   = llvm::BasicBlock::Create(ctx, "twLoopBody", fn);
        auto* exitBB   = llvm::BasicBlock::Create(ctx, "twLoopExit", fn);

        segmentCtx.builder.CreateBr(headerBB);
        segmentCtx.builder.SetInsertPoint(headerBB);

        llvm::Value* iVal = segmentCtx.builder.CreateLoad(i64Ty, iSlot);
        llvm::Value* cond = segmentCtx.builder.CreateICmpSLT(
            iVal, llvm::ConstantInt::get(i64Ty, windowsPerTuple));
        segmentCtx.builder.CreateCondBr(cond, bodyBB, exitBB);
        segmentCtx.builder.SetInsertPoint(bodyBB);

        llvm::Value* offset = segmentCtx.builder.CreateMul(iVal, slideVal);
        llvm::Value* windowStart = segmentCtx.builder.CreateSub(alignedStart, offset);
        llvm::Value* windowEnd   = segmentCtx.builder.CreateAdd(windowStart, sizeVal);

        llvm::Value* validStart = segmentCtx.builder.CreateICmpSGE(
            windowStart, llvm::ConstantInt::get(i64Ty, 0));
        llvm::Value* validEnd = segmentCtx.builder.CreateICmpSLT(ts, windowEnd);
        llvm::Value* valid = segmentCtx.builder.CreateAnd(validStart, validEnd);

        auto* emitBB = llvm::BasicBlock::Create(ctx, "twLoopEmit", fn);
        auto* contBB = llvm::BasicBlock::Create(ctx, "twLoopCont", fn);
        segmentCtx.builder.CreateCondBr(valid, emitBB, contBB);

        segmentCtx.builder.SetInsertPoint(emitBB);
        emitWindow(segmentCtx, tuple, windowStart, windowEnd, consumers);
        segmentCtx.builder.CreateBr(contBB);

        segmentCtx.builder.SetInsertPoint(contBB);
        llvm::Value* next = segmentCtx.builder.CreateAdd(iVal, llvm::ConstantInt::get(i64Ty, 1));
        segmentCtx.builder.CreateStore(next, iSlot);
        segmentCtx.builder.CreateBr(headerBB);

        segmentCtx.builder.SetInsertPoint(exitBB);
    }

    void TimeWindowOperator::codegenTumbling(core::physical::segment::SegmentContext& segmentCtx,
        const core::physical::data::DataPackage& tuple,
        const core::physical::segment::SegmentDataConsumers& consumers) const {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        auto* i64Ty = llvm::Type::getInt64Ty(ctx);

        llvm::Value* timestamp = tuple.timestamp;
        llvm::Value* sizeVal = llvm::ConstantInt::get(i64Ty, mConfig.size.count());

        //llvm based calculations to get the window data
        llvm::Value* k = segmentCtx.builder.CreateSDiv(timestamp, sizeVal);
        llvm::Value* windowStart = segmentCtx.builder.CreateMul(k, sizeVal);
        llvm::Value* windowEnd = segmentCtx.builder.CreateAdd(windowStart, sizeVal);

        emitWindow(segmentCtx, tuple, windowStart, windowEnd, consumers);
    }

    void TimeWindowOperator::consume(core::physical::segment::SegmentContext& segmentCtx,
                 core::shared::identifiers::PortId  /*inputPort*/,
                 const core::physical::data::DataPackage& tuple,
                 const core::physical::segment::SegmentDataConsumers& consumers) {
        const std::int64_t sizeMs  = mConfig.size.count();
        const std::int64_t slideMs = mConfig.slide.count();
        const std::int64_t windowsPerTuple = (sizeMs + slideMs - 1) / slideMs;

        //Threshold (not tunable!) to switch between unrolled and looped code execution
        static constexpr std::int64_t kUnrollThreshold = 16;

        if (windowsPerTuple == 1) {
            codegenTumbling(segmentCtx, tuple, consumers);
        } else if (windowsPerTuple <= kUnrollThreshold) {
            codegenUnrolled(segmentCtx, tuple, consumers, windowsPerTuple);
        } else {
            codegenLoop(segmentCtx, tuple, consumers, windowsPerTuple);
        }
    }

    void TimeWindowOperator::end(core::physical::segment::SegmentContext& /*segmentCtx*/,
             core::shared::identifiers::PortId  /*inputPort*/,
             const core::physical::segment::SegmentDataConsumers& /*consumers*/) {
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::physical