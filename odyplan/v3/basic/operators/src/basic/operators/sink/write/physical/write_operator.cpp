#include "basic/io/io_format.h"

#include <basic/operators/sink/write/physical/write_operator.h>

namespace sierra::odyplan::v3::basic::operators::sink::write::physical {
    WriteOperator::WriteOperator(const core::physical::port::Port& inputPort,
                                 const core::shared::identifiers::OperatorId lOpId,
                                 nlohmann::json ioConfig,
                                 const io::IOFormatType ioFormat,
                                 core::logical::port::Port dataSchema)
        : TypedSinkOperator(inputPort),
          mLogicalOperatorId(lOpId),
          mIOFormat(ioFormat),
          mDataSchema(std::move(dataSchema)),
          mIOConfig(std::move(ioConfig)) {
        if (mDataSchema.empty()) {
            throw std::invalid_argument("WriteOperator: dataSchema is empty");
        }
    }

    void
    WriteOperator::consume(core::physical::segment::SegmentContext& segmentCtx,
                           core::shared::identifiers::PortId /*inputPort*/,
                           const core::physical::data::DataPackage& tuple,
                           const core::physical::segment::SegmentDataConsumers& /*consumers*/) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto* voidTy = llvm::Type::getVoidTy(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);

        llvm::Value* state = loadStatePtr(segmentCtx);

        const auto beginFn = module->getOrInsertFunction(
            "rtIOSinkBeginData", llvm::FunctionType::get(voidTy, {ptrTy}, false));
        segmentCtx.builder.CreateCall(beginFn, {state});

        for (std::size_t i = 0; i < mDataSchema.size(); ++i) {
            const auto& dataField = mDataSchema.at(i);
            llvm::Value* fieldName = segmentCtx.builder.CreateGlobalString(
                dataField.fieldName, "field_" + dataField.fieldName, 0, module);
            codegenField(segmentCtx, state, fieldName, tuple.at(i), dataField.valueTypeId);
        }

        const auto endFn = module->getOrInsertFunction(
            "rtIOSinkEndData", llvm::FunctionType::get(voidTy, {ptrTy}, false));
        segmentCtx.builder.CreateCall(endFn, {state});
    }

    void WriteOperator::declareState(core::physical::state::StateRegistry& stateRegistry,
                                     llvm::Module& module,
                                     llvm::LLVMContext& context) {
        auto* i8Ty = llvm::Type::getInt8Ty(context);
        auto* ptrTy = llvm::PointerType::getUnqual(context);

        const std::string path = mIOConfig.at("path").get<std::string>();

        auto* strConstant = llvm::ConstantDataArray::getString(context, path, true);

        // NOLINTBEGIN(cppcoreguidelines-owning-memory,clang-analyzer-cplusplus.NewDeleteLeaks)
        auto* gPath = new llvm::GlobalVariable(module,
                                               strConstant->getType(),
                                               true,
                                               llvm::GlobalValue::ExternalLinkage,
                                               strConstant,
                                               "write_path" + std::to_string(mLogicalOperatorId));

        llvm::Value* pathPtr = llvm::ConstantExpr::getInBoundsGetElementPtr(
            gPath->getValueType(),
            gPath,
            std::vector<llvm::Value*>{llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), 0),
                                      llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), 0)});
        // NOLINTEND(cppcoreguidelines-owning-memory,clang-analyzer-cplusplus.NewDeleteLeaks)

        mState = stateRegistry.createHandle(
            "write_sink_" + std::to_string(mLogicalOperatorId),
            "rtIOSinkCreate",
            {llvm::ConstantInt::get(i8Ty, static_cast<std::uint8_t>(mIOFormat)), pathPtr},
            {i8Ty, ptrTy},
            "rtIOSinkDestroy");
    }

    llvm::Value*
    WriteOperator::loadStatePtr(core::physical::segment::SegmentContext& segmentCtx) const {
        auto* ptrTy = llvm::PointerType::getUnqual(segmentCtx.builder.getContext());
        return segmentCtx.builder.CreateLoad(ptrTy, mState.var);
    }

    void WriteOperator::end(core::physical::segment::SegmentContext& segmentCtx,
                            core::shared::identifiers::PortId /*inputPort*/,
                            const core::physical::segment::SegmentDataConsumers& /*consumers*/) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto* voidTy = llvm::Type::getVoidTy(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);

        llvm::Value* state = loadStatePtr(segmentCtx);
        const auto flushFn = module->getOrInsertFunction(
            "rtIOSinkFlush", llvm::FunctionType::get(voidTy, {ptrTy}, false));
        segmentCtx.builder.CreateCall(flushFn, {state});
    }

    void WriteOperator::codegenField(core::physical::segment::SegmentContext& segmentCtx,
                                     llvm::Value* state,
                                     llvm::Value* fieldName,
                                     llvm::Value* fieldValue,
                                     const core::shared::types::ValueTypeId typeId) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto* voidTy = llvm::Type::getVoidTy(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);
        auto* typeIdTy = llvm::Type::getInt64Ty(ctx);

        llvm::Value* ptrFieldValue = nullptr;

        if (fieldValue->getType()->isPointerTy()) {
            ptrFieldValue = fieldValue;
        } else {
            auto *slot = segmentCtx.storage.getNamed("slot." +std::to_string(getPId()), typeId);
            segmentCtx.builder.CreateStore(fieldValue, slot);
            ptrFieldValue = slot;
        }

        const auto fn = module->getOrInsertFunction(
            "rtIOSinkWrite",
            llvm::FunctionType::get(voidTy, {ptrTy, typeIdTy, ptrTy, ptrTy}, false));

        segmentCtx.builder.CreateCall(
            fn, {state, llvm::ConstantInt::get(typeIdTy, typeId), fieldName, ptrFieldValue});
    }
}  // namespace sierra::odyplan::v3::basic::operators::sink::write::physical
