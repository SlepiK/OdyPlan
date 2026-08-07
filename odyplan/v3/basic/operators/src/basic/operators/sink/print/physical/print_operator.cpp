#include <basic/operators/sink/print/physical/print_operator.h>

namespace sierra::odyplan::v3::basic::operators::sink::print::physical {
    PrintOperator::PrintOperator(core::physical::port::Port inputPort, PortFieldIndexV indices)
        : TypedSinkOperator(std::move(inputPort)),
          mPortFieldIndices(std::move(indices)) {
    }

    void PrintOperator::consume(core::physical::segment::SegmentContext &segmentCtx,
                                core::shared::identifiers::PortId /*inputPort*/,
                                const core::physical::data::DataPackage &tuple,
                                const core::physical::segment::SegmentDataConsumers & /*consumers*/) {
        llvm::Module *module = segmentCtx.builder.GetInsertBlock()->getModule();
        if (tuple.size() < mPortFieldIndices.size()) {
            throw std::out_of_range("PrintOperator: given field indices dont match incoming tuple");
        }

        codegenPrintTimestamp(segmentCtx.builder, module, tuple.timestamp);

        codegenPrint(segmentCtx.builder, module, "rtOdyPrintSeperator");

        for (std::size_t i = 0; i < mPortFieldIndices.size(); ++i) {
            std::size_t const fieldIndex = mPortFieldIndices.at(i);
            codegenPrintField(segmentCtx.builder, module, getInputPorts().at(0).at(fieldIndex).valueTypeId,
                              tuple.at(fieldIndex));

            if (i + 1 < mPortFieldIndices.size()) {
                codegenPrint(segmentCtx.builder, module, "rtOdyPrintSeperator");
            }
        }
        codegenPrint(segmentCtx.builder, module, "rtOdyPrintNewline");
    }

    void PrintOperator::end(core::physical::segment::SegmentContext &segmentCtx,
                            const core::shared::identifiers::PortId inputPort,
                            const core::physical::segment::SegmentDataConsumers &consumers) {
    }

    void PrintOperator::codegenPrint(llvm::IRBuilder<> &builder, llvm::Module *module, const char *name) {
        auto *fnType = llvm::FunctionType::get(
            llvm::Type::getVoidTy(builder.getContext()),
            {},
            false
        );
        builder.CreateCall(module->getOrInsertFunction(name, fnType), {});
    }

    void PrintOperator::codegenPrintTimestamp(llvm::IRBuilder<> &builder, llvm::Module *module, llvm::Value *value) {
        llvm::LLVMContext &ctx = builder.getContext();
        auto *i64Ty = llvm::Type::getInt64Ty(ctx);
        auto *fnType = llvm::FunctionType::get(
            llvm::Type::getVoidTy(ctx),
            {i64Ty},
            false
        );
        llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintTimeStamp", fnType);
        builder.CreateCall(fn, {value});
    }

    void PrintOperator::codegenPrintField(llvm::IRBuilder<> &builder, llvm::Module *module,
                                          const core::shared::types::ValueTypeId typeId, llvm::Value *value) {
        llvm::LLVMContext &ctx = builder.getContext();
        switch (static_cast<core::shared::types::BuiltinValueType>(typeId)) {
            case core::shared::types::BuiltinValueType::BOOL: {
                auto *fnType = llvm::FunctionType::get(
                    llvm::Type::getVoidTy(ctx),
                    {value->getType()},
                    false
                );
                llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintInt", fnType);
                builder.CreateCall(fn, {value});
                break;
            }
            case core::shared::types::BuiltinValueType::INT8: {
                auto *fnType = llvm::FunctionType::get(
                    llvm::Type::getVoidTy(ctx),
                    {value->getType()},
                    false
                );
                llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintInt", fnType);
                builder.CreateCall(fn, {value});
                break;
            }
            case core::shared::types::BuiltinValueType::INT16: {
                auto *fnType = llvm::FunctionType::get(
                    llvm::Type::getVoidTy(ctx),
                    {value->getType()},
                    false
                );
                llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintInt", fnType);
                builder.CreateCall(fn, {value});
                break;
            }
            case core::shared::types::BuiltinValueType::INT32: {
                auto *fnType = llvm::FunctionType::get(
                    llvm::Type::getVoidTy(ctx),
                    {value->getType()},
                    false
                );
                llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintInt", fnType);
                builder.CreateCall(fn, {value});
                break;
            }
            case core::shared::types::BuiltinValueType::INT64: {
                auto *fnType = llvm::FunctionType::get(
                    llvm::Type::getVoidTy(ctx),
                    {value->getType()},
                    false
                );
                llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintInt", fnType);
                builder.CreateCall(fn, {value});
                break;
            }
            case core::shared::types::BuiltinValueType::FLOAT: {
                auto *fnType = llvm::FunctionType::get(
                    llvm::Type::getVoidTy(ctx),
                    {value->getType()},
                    false
                );
                llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintFloat", fnType);
                builder.CreateCall(fn, {value});
                break;
            }
            case core::shared::types::BuiltinValueType::DOUBLE: {
                auto *fnType = llvm::FunctionType::get(
                    llvm::Type::getVoidTy(ctx),
                    {value->getType()},
                    false
                );
                llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintDouble", fnType);
                builder.CreateCall(fn, {value});
                break;
            }
            case core::shared::types::BuiltinValueType::STRING: {
                auto *fnType = llvm::FunctionType::get(
                    llvm::Type::getVoidTy(ctx),
                    {llvm::PointerType::getUnqual(ctx)},
                    false
                );
                llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintString", fnType);
                builder.CreateCall(fn, {value});
                break;
            }
            default:
                throw std::logic_error("PrintOperator: non-printable type reached");
        }
    }
}