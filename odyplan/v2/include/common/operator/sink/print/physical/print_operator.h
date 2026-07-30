#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_PRINT_OPERATOR_PHYSICAL_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_PRINT_OPERATOR_PHYSICAL_H

#include <llvm/IR/Module.h>
#include "core/common/identifier/type.h"
#include "core/physical/operator/sink/sink_operator.h"

namespace sierra::odyplan::v2::common::opr::sink::physical {
    class PrintOperator : public core::physical::opr::TypedSinkOperator {
    public:
        using PortFieldIndexV = std::vector<std::size_t>;

        explicit PrintOperator(core::physical::port::Port inputPort, PortFieldIndexV indices)
            : TypedSinkOperator(std::move(inputPort)),
              mPortFieldIndices(std::move(indices)) {
        }

        void consume(core::physical::segment::SegmentContext &segmentCtx,
                     core::common::identifier::PortId /*inputPort*/, const core::physical::port::PortTuple &tuple,
                     const core::physical::port::PortTupleConsumers & /*consumers*/) override {
            llvm::Module *module = segmentCtx.builder.GetInsertBlock()->getModule();
            if (tuple.size() < mPortFieldIndices.size()) {
                throw std::out_of_range("PrintOperator: given field indices dont match incoming tuple");
            }

            codegenPrintTimestamp(segmentCtx.builder, module, tuple.timestamp);

            codegenPrint(segmentCtx.builder, module, "rtOdyPrintSeperator");

            for (std::size_t i = 0; i < mPortFieldIndices.size(); ++i) {
                std::size_t const fieldIndex = mPortFieldIndices.at(i);
                codegenPrintField(segmentCtx.builder, module, getInputPorts().at(0).at(fieldIndex).typeId,
                                  tuple.at(fieldIndex));

                if (i + 1 < mPortFieldIndices.size()) {
                    codegenPrint(segmentCtx.builder, module, "rtOdyPrintSeperator");
                }
            }
            codegenPrint(segmentCtx.builder, module, "rtOdyPrintNewline");
        }

        void end(core::physical::segment::SegmentContext &segmentCtx, const core::common::identifier::PortId inputPort,
                 const core::physical::port::PortTupleConsumers &consumers) override {
        }

    private:
        PortFieldIndexV mPortFieldIndices;

        static void codegenPrint(llvm::IRBuilder<> &builder, llvm::Module *module, const char *name) {
            auto *fnType = llvm::FunctionType::get(
                llvm::Type::getVoidTy(builder.getContext()),
                {},
                false
            );
            builder.CreateCall(module->getOrInsertFunction(name, fnType), {});
        }

        static void codegenPrintTimestamp(llvm::IRBuilder<> &builder, llvm::Module *module, llvm::Value *value) {
            llvm::LLVMContext &ctx = builder.getContext();
            auto* i64Ty = llvm::Type::getInt64Ty(ctx);
            auto *fnType = llvm::FunctionType::get(
                        llvm::Type::getVoidTy(ctx),
                        {i64Ty},
                        false
                        );
            llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintTimeStamp", fnType);
            builder.CreateCall(fn, {value});
        }

        static void codegenPrintField(llvm::IRBuilder<> &builder, llvm::Module *module,
                                      const core::common::identifier::TypeId typeId, llvm::Value *value) {
            llvm::LLVMContext &ctx = builder.getContext();
            switch (static_cast<core::common::identifier::Type>(typeId)) {
                case core::common::identifier::Type::BOOL: {
                    auto *fnType = llvm::FunctionType::get(
                        llvm::Type::getVoidTy(ctx),
                        {value->getType()},
                        false
                    );
                    llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintInt", fnType);
                    builder.CreateCall(fn, {value});
                    break;
                }
                case core::common::identifier::Type::INT8: {
                    auto *fnType = llvm::FunctionType::get(
                        llvm::Type::getVoidTy(ctx),
                        {value->getType()},
                        false
                    );
                    llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintInt", fnType);
                    builder.CreateCall(fn, {value});
                    break;
                }
                case core::common::identifier::Type::INT16: {
                    auto *fnType = llvm::FunctionType::get(
                        llvm::Type::getVoidTy(ctx),
                        {value->getType()},
                        false
                    );
                    llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintInt", fnType);
                    builder.CreateCall(fn, {value});
                    break;
                }
                case core::common::identifier::Type::INT32: {
                    auto *fnType = llvm::FunctionType::get(
                        llvm::Type::getVoidTy(ctx),
                        {value->getType()},
                        false
                    );
                    llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintInt", fnType);
                    builder.CreateCall(fn, {value});
                    break;
                }
                case core::common::identifier::Type::INT64: {
                    auto *fnType = llvm::FunctionType::get(
                        llvm::Type::getVoidTy(ctx),
                        {value->getType()},
                        false
                    );
                    llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintInt", fnType);
                    builder.CreateCall(fn, {value});
                    break;
                }
                case core::common::identifier::Type::FLOAT: {
                    auto *fnType = llvm::FunctionType::get(
                        llvm::Type::getVoidTy(ctx),
                        {value->getType()},
                        false
                    );
                    llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintFloat", fnType);
                    builder.CreateCall(fn, {value});
                    break;
                }
                case core::common::identifier::Type::DOUBLE: {
                    auto *fnType = llvm::FunctionType::get(
                        llvm::Type::getVoidTy(ctx),
                        {value->getType()},
                        false
                    );
                    llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintDouble", fnType);
                    builder.CreateCall(fn, {value});
                    break;
                }
                case core::common::identifier::Type::STRING: {
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
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_PRINT_OPERATOR_PHYSICAL_H