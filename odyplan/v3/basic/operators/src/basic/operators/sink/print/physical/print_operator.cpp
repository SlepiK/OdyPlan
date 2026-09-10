#include <basic/operators/sink/print/physical/print_operator.h>

namespace sierra::odyplan::v3::basic::operators::sink::print::physical {
    PrintOperator::PrintOperator(core::physical::port::Port inputPort, PortFieldIndexV indices, core::logical::port::Port lInputPort)
        : TypedSinkOperator(std::move(inputPort)), mPortFieldIndices(std::move(indices)), mSchema(std::move(lInputPort)) {}

    void
    PrintOperator::consume(core::physical::segment::SegmentContext& segmentCtx,
                           core::shared::identifiers::PortId /*inputPort*/,
                           const core::physical::data::DataPackage& tuple,
                           const core::physical::segment::SegmentDataConsumers& /*consumers*/) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto* voidTy = llvm::Type::getVoidTy(ctx);
        auto* i64Ty = llvm::Type::getInt64Ty(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);
        auto* i8Ty = llvm::Type::getInt8Ty(ctx);

        const std::size_t fieldCount = mPortFieldIndices.size();

        llvm::StructType* printFieldTy = llvm::StructType::get(ctx, {
            ptrTy, //value
            ptrTy, //name
            i8Ty //typeId
        });
        auto* printFieldArrayTy = llvm::ArrayType::get(printFieldTy, fieldCount);

        llvm::Value* printFieldArray = segmentCtx.builder.CreateAlloca(printFieldArrayTy);

        for (std::size_t i = 0; i < fieldCount; ++i) {
            const auto fieldIndex = mPortFieldIndices.at(i);
            const auto& fieldDesc = mSchema.at(fieldIndex);

            llvm::Value* fieldValue = tuple.at(fieldIndex);
            llvm::Value* valuePtr;
            if (fieldValue->getType()->isPointerTy()) {
                valuePtr = fieldValue;
            } else {
                llvm::Value* valueSlot = segmentCtx.builder.CreateAlloca(fieldValue->getType());
                segmentCtx.builder.CreateStore(fieldValue, valueSlot);
                valuePtr = valueSlot;
            }

            llvm::Value* fieldTypeId = llvm::ConstantInt::get(i8Ty, fieldDesc.valueTypeId);

            llvm::Constant* fieldName = segmentCtx.builder.CreateGlobalString(fieldDesc.fieldName, "field_name" +std::to_string(getPId()) + "_" + std::to_string(fieldIndex));

            llvm::Value* printFieldPtr = segmentCtx.builder.CreateGEP(printFieldArrayTy, printFieldArray, {llvm::ConstantInt::get(i64Ty, 0), llvm::ConstantInt::get(i64Ty, i)});

            segmentCtx.builder.CreateStore(segmentCtx.builder.CreateBitCast(valuePtr, ptrTy), segmentCtx.builder.CreateStructGEP(printFieldTy, printFieldPtr, 0));
            segmentCtx.builder.CreateStore(fieldName, segmentCtx.builder.CreateStructGEP(printFieldTy, printFieldPtr, 1));
            segmentCtx.builder.CreateStore(fieldTypeId, segmentCtx.builder.CreateStructGEP(printFieldTy, printFieldPtr, 2));
        }

        auto* printContextTy = llvm::StructType::get(ctx, {
            ptrTy,
            i64Ty,
            i64Ty,
            i64Ty
        });
        llvm::Value* printContext = segmentCtx.builder.CreateAlloca(printContextTy);

        llvm::Value* fieldsPtr = segmentCtx.builder.CreateGEP(printFieldArrayTy, printFieldArray, {llvm::ConstantInt::get(i64Ty, 0), llvm::ConstantInt::get(i64Ty, 0)});

        segmentCtx.builder.CreateStore(fieldsPtr, segmentCtx.builder.CreateStructGEP(printContextTy, printContext, 0));
        segmentCtx.builder.CreateStore(llvm::ConstantInt::get(i64Ty, fieldCount), segmentCtx.builder.CreateStructGEP(printContextTy, printContext, 1));
        segmentCtx.builder.CreateStore(tuple.timeInterval.getStart().getPoint(), segmentCtx.builder.CreateStructGEP(printContextTy, printContext, 2));
        segmentCtx.builder.CreateStore(tuple.timeInterval.getEnd().getPoint(), segmentCtx.builder.CreateStructGEP(printContextTy, printContext, 3));

        const auto printFn = module->getOrInsertFunction("rtPrintDataPackage", llvm::FunctionType::get(voidTy, {ptrTy}, false));
        segmentCtx.builder.CreateCall(printFn, {printContext});
    }

    void PrintOperator::end(core::physical::segment::SegmentContext& segmentCtx,
                            const core::shared::identifiers::PortId inputPort,
                            const core::physical::segment::SegmentDataConsumers& consumers) {}

    void PrintOperator::consumePunctuation(core::physical::segment::SegmentContext &segmentCtx,
                                core::shared::identifiers::PortId inputPort,
                                const core::physical::data::PunctuationPackage &punctuation,
                                const core::physical::segment::SegmentDataConsumers &consumers) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        const auto printFn = module->getOrInsertFunction("rtOdyPrintPunctuation",
                                                         llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
                                                             {llvm::Type::getInt64Ty(ctx), llvm::Type::getInt1Ty(ctx)},
                                                             false));

        segmentCtx.builder.CreateCall(printFn, {punctuation.watermark, punctuation.isFinal});
    }
}  // namespace sierra::odyplan::v3::basic::operators::sink::print::physical