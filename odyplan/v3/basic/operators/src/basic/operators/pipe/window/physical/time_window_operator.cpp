#include <basic/operators/pipe/window/physical/time_window_operator.h>
#include <llvm/CodeGenTypes/MachineValueType.h>

#include "spdlog/fmt/bundled/format.h"

namespace sierra::odyplan::v3::basic::operators::pipe::window::physical {
    namespace {
        basic::window::TimeWindowType resolveTimeWindowType(const basic::window::time::TimeWindowConfig& config ) {
            if (!config.slide.has_value() && !config.advance.has_value()) {
                return basic::window::TimeWindowType::SLIDING;
            }
            if (!config.slide.has_value()) {
                return basic::window::TimeWindowType::ADVANCE;
            }
            return basic::window::TimeWindowType::PERIODIC;
        }
    }

    TimeWindowOperator::TimeWindowOperator(core::physical::port::Port inputPort,
                                           core::physical::port::Port outputPort,
                                           const basic::window::time::TimeWindowConfig& config)
    : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)},
                        core::physical::port::Ports{std::move(outputPort)}),
          mConfig(config), mWindowType(resolveTimeWindowType(config)) {
        if (mWindowType == basic::window::TimeWindowType::PERIODIC) {
            throw std::logic_error("TimeWindowOperator: not implemented window_type periodic used");
        }

    }

    void TimeWindowOperator::consume(core::physical::segment::SegmentContext &segmentCtx,
                                     core::shared::identifiers::PortId inputPort,
                                     const core::physical::data::DataPackage &tuple,
                                     const core::physical::segment::SegmentDataConsumers &consumers) {
        llvm::LLVMContext &ctx = segmentCtx.builder.getContext();
        llvm::Module *module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto *i64Ty = llvm::Type::getInt64Ty(ctx);

        llvm::Function *smaxFn = llvm::Intrinsic::getOrInsertDeclaration(module, llvm::Intrinsic::smax, {i64Ty});
        llvm::Value *eventTs = segmentCtx.builder.CreateCall(
            smaxFn, {tuple.timeInterval.getStart().getPoint(), llvm::ConstantInt::get(i64Ty, 0)});

        llvm::Value* windowEnd = computeWindowEnd(segmentCtx, eventTs);

        llvm::Value* isValid = segmentCtx.builder.CreateICmpSGT(windowEnd, eventTs);

        llvm::Function* fn = segmentCtx.builder.GetInsertBlock()->getParent();
        auto* emitBB = llvm::BasicBlock::Create(ctx, "time_window_emit", fn);
        auto* mergeBB = llvm::BasicBlock::Create(ctx, "time_window_merge", fn);
        segmentCtx.builder.CreateCondBr(isValid, emitBB, mergeBB);

        segmentCtx.builder.SetInsertPoint(emitBB);

        core::physical::data::DataPackage emit = tuple;
        emit.timeInterval = core::physical::time::TimeInterval(segmentCtx.builder, tuple.timeInterval.getStart(), core::physical::time::TimePoint(segmentCtx.builder, windowEnd));

        consumers.front().consume(segmentCtx, emit);

        segmentCtx.builder.CreateBr(mergeBB);

        segmentCtx.builder.SetInsertPoint(mergeBB);
    }

    llvm::Value* TimeWindowOperator::computeWindowEnd(core::physical::segment::SegmentContext& segmentCtx, llvm::Value* eventTs) const {
        llvm::LLVMContext &ctx = segmentCtx.builder.getContext();

        auto* i64Ty = llvm::Type::getInt64Ty(ctx);

        llvm::Value* sizeValue = llvm::ConstantInt::get(i64Ty, mConfig.size);

        if (mWindowType == basic::window::TimeWindowType::ADVANCE) {
            llvm::Value* advanceValue = llvm::ConstantInt::get(i64Ty, *mConfig.advance);
            llvm::Value* floor = segmentCtx.builder.CreateSDiv(eventTs, advanceValue);
            llvm::Value* alignedStart = segmentCtx.builder.CreateMul(floor, advanceValue);
            return segmentCtx.builder.CreateAdd(alignedStart, sizeValue);
        }

        return segmentCtx.builder.CreateAdd(eventTs, sizeValue);
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::physical