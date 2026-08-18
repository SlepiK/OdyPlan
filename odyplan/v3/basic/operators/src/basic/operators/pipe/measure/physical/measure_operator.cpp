#include <basic/operators/pipe/measure/physical/measure_operator.h>

#include "basic/operators/pipe/measure/runtime/details/measure_state.h"

namespace sierra::odyplan::v3::basic::operators::pipe::measure::physical {
    MeasureOperator::MeasureOperator(core::physical::port::Port inputPort,
                                     core::physical::port::Port outputPort,
                                     std::string label)
        : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)},
                            core::physical::port::Ports{std::move(outputPort)}),
          mLabel(std::move(label)) {}

    void MeasureOperator::consume(core::physical::segment::SegmentContext& segmentCtx,
                                  core::shared::identifiers::PortId /*inputPort*/,
                                  const core::physical::data::DataPackage& tuple,
                                  const core::physical::segment::SegmentDataConsumers& consumers) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();
        auto* voidTy = llvm::Type::getVoidTy(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);

        llvm::Value* state = loadStatePtr(segmentCtx);

        const auto tickFn = module->getOrInsertFunction("rtMeasureTick",
                                                  llvm::FunctionType::get(voidTy, {ptrTy}, false));
        segmentCtx.builder.CreateCall(tickFn, {state});

        consumers.at(0)(segmentCtx, tuple);
    }

    void MeasureOperator::end(core::physical::segment::SegmentContext& segmentCtx,
                              const core::shared::identifiers::PortId /*inputPort*/,
                              const core::physical::segment::SegmentDataConsumers& /*consumers*/) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();
        auto* voidTy = llvm::Type::getVoidTy(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);

        llvm::Value* state = loadStatePtr(segmentCtx);
        const auto finalFn = module->getOrInsertFunction(
            "rtMeasureFinalReport", llvm::FunctionType::get(voidTy, {ptrTy}, false));
        segmentCtx.builder.CreateCall(finalFn, {state});
    }

    void MeasureOperator::declareState(core::physical::state::StateRegistry& stateRegistry,
                                       llvm::Module& module,
                                       llvm::LLVMContext& context) {
        auto* ptrTy = llvm::PointerType::getUnqual(context);
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,clang-analyzer-cplusplus.NewDeleteLeaks)
        auto* labelGlobalVariable = new llvm::GlobalVariable(
            module,
            llvm::ArrayType::get(llvm::Type::getInt8Ty(context), mLabel.size() + 1),
            true,
            llvm::GlobalValue::InternalLinkage,
            llvm::ConstantDataArray::getString(context, mLabel, true));

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,clang-analyzer-cplusplus.NewDeleteLeaks)
        llvm::Value* labelGlobal = llvm::ConstantExpr::getInBoundsGetElementPtr(
            llvm::ArrayType::get(llvm::Type::getInt8Ty(context), mLabel.size() + 1),
            labelGlobalVariable,
            std::vector<llvm::Value*>{llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), 0),
                                      llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), 0)});

        mStateHandle = stateRegistry.createHandle("measure_" + std::to_string(getPId()),
                                                  "rtCreateMeasure",
                                                  {labelGlobal},
                                                  {ptrTy},
                                                  "rtDestroyMeasure");
    }

    llvm::Value*
    MeasureOperator::loadStatePtr(core::physical::segment::SegmentContext& segmentCtx) const {
        auto* ptrTy = llvm::PointerType::getUnqual(segmentCtx.builder.getContext());
        return segmentCtx.builder.CreateLoad(ptrTy, mStateHandle.var);
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::measure::physical