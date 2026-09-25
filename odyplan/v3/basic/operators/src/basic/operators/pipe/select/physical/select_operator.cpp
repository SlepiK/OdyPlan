#include <basic/operators/pipe/select/physical/select_operator.h>
#include <stdexcept>

namespace sierra::odyplan::v3::basic::operators::pipe::select::physical {
    SelectOperator::SelectOperator(core::physical::port::Port inputPort,
                                   core::physical::port::Port outputPort,
                                   std::unique_ptr<basic::expression::physical::IExpression> predicate)
        : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)},
                            core::physical::port::Ports{std::move(outputPort)}),
          mPredicate(std::move(predicate)) {}

    void SelectOperator::consume(core::physical::segment::SegmentContext& segmentCtx,
                                 core::shared::identifiers::PortId /*inputPort*/,
                                 const core::physical::data::DataPackage& tuple,
                                 const core::physical::segment::SegmentDataConsumers& consumers) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Function* fn = segmentCtx.builder.GetInsertBlock()->getParent();

        llvm::Value* cnd = expression::physical::codegenExpression(segmentCtx, tuple, *mPredicate);

        auto* passBB = llvm::BasicBlock::Create(ctx, "select_pass", fn);
        auto* watermarkBB = llvm::BasicBlock::Create(ctx, "select_watermark", fn);
        auto* mergeBB = llvm::BasicBlock::Create(ctx, "select_merge", fn);

        segmentCtx.builder.CreateCondBr(cnd, passBB, watermarkBB);

        segmentCtx.builder.SetInsertPoint(passBB);
        consumers.front().consume(segmentCtx, tuple);
        segmentCtx.builder.CreateBr(mergeBB);

        segmentCtx.builder.SetInsertPoint(watermarkBB);

        const auto watermark = core::physical::data::PunctuationPackage::createHeartbeat(segmentCtx.builder, tuple.timeInterval.getStart().getPoint());

        consumers.front().consumePunctuation(segmentCtx, watermark);
        segmentCtx.builder.CreateBr(mergeBB);

        segmentCtx.builder.SetInsertPoint(mergeBB);
    }

    void SelectOperator::end(core::physical::segment::SegmentContext& segmentCtx,
                             const core::shared::identifiers::PortId inputPort,
                             const core::physical::segment::SegmentDataConsumers& consumers) {}

}  // namespace sierra::odyplan::v3::basic::operators::pipe::select::physical