#include <basic/operators/sink/write/physical/write_operator.h>

namespace sierra::odyplan::v3::basic::operators::sink::write::physical {
    WriteOperator::WriteOperator(const core::physical::port::Port &inputPort, const io::IOSinkId ioSinkId,
                                 core::logical::port::Port dataSchema)
        : TypedSinkOperator(inputPort),
          mIOSinkId(ioSinkId),
          mDataSchema(std::move(dataSchema)) {
        if (mDataSchema.empty()) {
            throw std::invalid_argument("WriteOperator: dataSchema is empty");
        }
    }

    void WriteOperator::consume(core::physical::segment::SegmentContext &segmentCtx,
                                core::shared::identifiers::PortId /*inputPort*/,
                                const core::physical::data::DataPackage &tuple,
                                const core::physical::segment::SegmentDataConsumers & /*consumers*/) {
        llvm::LLVMContext &ctx = segmentCtx.builder.getContext();
        llvm::Module *module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto *voidTy = llvm::Type::getVoidTy(ctx);
        auto *idTy = llvm::Type::getInt64Ty(ctx);
        llvm::Value *index = llvm::ConstantInt::get(idTy, mIOSinkId);

        const auto beginFn = module->getOrInsertFunction("rtIOSinkBeginData",
                                                         llvm::FunctionType::get(voidTy, {idTy}, false));
        segmentCtx.builder.CreateCall(beginFn, {index});

        for (std::size_t i = 0; i < mDataSchema.size(); ++i) {
            const auto &dataField = mDataSchema.at(i);
            llvm::Value *fieldName = segmentCtx.builder.CreateGlobalString(
                dataField.fieldName, "field_" + dataField.fieldName, 0, module);
            codegenField(segmentCtx, index, fieldName, tuple.at(i), dataField.valueTypeId);
        }

        const auto endFn = module->getOrInsertFunction("rtIOSinkEndData",
                                                       llvm::FunctionType::get(voidTy, {idTy}, false));
        segmentCtx.builder.CreateCall(endFn, {index});
    }

    void WriteOperator::end(core::physical::segment::SegmentContext &segmentCtx,
                            core::shared::identifiers::PortId inputPort,
                            const core::physical::segment::SegmentDataConsumers &consumers) {
    }

    void WriteOperator::codegenField(core::physical::segment::SegmentContext &segmentCtx, llvm::Value *index,
                                     llvm::Value *fieldName, llvm::Value *fieldValue,
                                     const core::shared::types::ValueTypeId typeId) {
        llvm::LLVMContext &ctx = segmentCtx.builder.getContext();
        llvm::Module *module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto *voidTy = llvm::Type::getVoidTy(ctx);
        auto *ptrTy = llvm::PointerType::getUnqual(ctx);
        auto *idTy = llvm::Type::getInt64Ty(ctx);
        auto *typeIdTy = llvm::Type::getInt64Ty(ctx);

        llvm::Value *ptrFieldValue = nullptr;

        if (fieldValue->getType()->isPointerTy()) {
            ptrFieldValue = fieldValue;
        } else {
            auto *slot = segmentCtx.storage.get(typeId);
            segmentCtx.builder.CreateStore(fieldValue, slot);
            ptrFieldValue = slot;
        }

        const auto fn = module->getOrInsertFunction(
            "rtIOSinkWrite",
            llvm::FunctionType::get(voidTy, {idTy, typeIdTy, ptrTy, ptrTy}, false)
        );

        segmentCtx.builder.CreateCall(fn, {index, llvm::ConstantInt::get(idTy, typeId), fieldName, ptrFieldValue});
    }
}