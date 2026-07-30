#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SOURCE_READ_OPERATOR_PHYSICAL_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SOURCE_READ_OPERATOR_PHYSICAL_H
#include <llvm/IR/Module.h>
#include <nlohmann/json.hpp>

#include "common/io/io_source.h"
#include "core/common/type/type_registry.h"
#include "core/logical/port/port.h"
#include "core/physical/operator/source/source_operator.h"

namespace sierra::odyplan::v2::common::opr::source::physical {
    class ReadOperator : public core::physical::opr::TypedSourceOperator {
    public:
        ReadOperator(core::physical::port::Port outputPort, const io::IOSourceId ioSourceId,
                     core::logical::port::Port dataSchema, std::optional<std::size_t> timestampFieldIndex = std::nullopt,
                     std::chrono::milliseconds rowDelay = std::chrono::milliseconds(0))
            : TypedSourceOperator(std::move(outputPort)),
              mIOSourceId(ioSourceId),
              mDataSchema(std::move(dataSchema)),
                mTimestampFieldIndex(timestampFieldIndex),
                mRowDelay(rowDelay) {
            if (mDataSchema.empty()) {
                throw std::invalid_argument("ReadOperator: dataSchema is empty");
            }
            if (mTimestampFieldIndex.has_value() && *mTimestampFieldIndex >= mDataSchema.size()) {
                throw std::out_of_range("ReadOperator: timestamp field index out of range");
            }
            if (mTimestampFieldIndex.has_value() && mDataSchema.at(*mTimestampFieldIndex).typeId != std::to_underlying(core::common::identifier::Type::INT64)) {
                throw std::invalid_argument("ReadOperator: timestamp field wrong type");
            }
        }

        void produce(core::physical::segment::SegmentContext &segmentCtx,
                     const core::physical::port::PortTupleConsumers &consumers) override {
            llvm::LLVMContext &ctx = segmentCtx.builder.getContext();
            llvm::Module *module = segmentCtx.builder.GetInsertBlock()->getModule();
            llvm::Function *fn = segmentCtx.builder.GetInsertBlock()->getParent();

            auto *ptrTy = llvm::PointerType::getUnqual(ctx);
            auto *i1Ty = llvm::Type::getInt1Ty(ctx);
            auto *idTy = llvm::Type::getInt64Ty(ctx);

            llvm::Value *sourceId = llvm::ConstantInt::get(idTy, mIOSourceId);

            auto *loopBB = llvm::BasicBlock::Create(ctx, "read_loop", fn);
            auto *produceBB = llvm::BasicBlock::Create(ctx, "read_produce", fn);
            auto *exitBB = llvm::BasicBlock::Create(ctx, "read_exit", fn);

            segmentCtx.builder.CreateBr(loopBB);
            segmentCtx.builder.SetInsertPoint(loopBB);

            const auto hasNextFn = module->getOrInsertFunction("rtIOSourceHasNext",
                                                               llvm::FunctionType::get(i1Ty, {idTy}, false));
            llvm::Value *hasNext = segmentCtx.builder.CreateCall(hasNextFn, {sourceId});

            if (mRowDelay.count() > 0) {
                auto* i64Ty = llvm::Type::getInt64Ty(ctx);
                auto* voidTy = llvm::Type::getVoidTy(ctx);
                const auto sleepFn = module->getOrInsertFunction("rtSleepMs", llvm::FunctionType::get(voidTy, {i64Ty}, false));
                segmentCtx.builder.CreateCall(sleepFn, {llvm::ConstantInt::get(i64Ty, mRowDelay.count())});
            }

            const auto stopFn = module->getOrInsertFunction("odyStop", llvm::FunctionType::get(i1Ty, {}, false));
            llvm::Value *stopped = segmentCtx.builder.CreateCall(stopFn, {});
            llvm::Value *notStopped = segmentCtx.builder.CreateNot(stopped);
            llvm::Value *shouldContinue = segmentCtx.builder.CreateAnd(hasNext, notStopped);

            segmentCtx.builder.CreateCondBr(shouldContinue, produceBB, exitBB);

            segmentCtx.builder.SetInsertPoint(produceBB);

            const auto getFieldFn = module->getOrInsertFunction("rtIOSourceGetField",
                                                                llvm::FunctionType::get(ptrTy, {idTy, ptrTy}, false));

            core::physical::port::PortTuple tuple;
            tuple.reserve(mDataSchema.size());

            for (const auto &dataField: mDataSchema) {
                llvm::Value *fieldName = segmentCtx.builder.CreateGlobalString(
                    dataField.fieldName, "field_" + dataField.fieldName, 0, module);
                llvm::Value *strValue = segmentCtx.builder.CreateCall(getFieldFn, {sourceId, fieldName});
                tuple.pushBack(codegenField(segmentCtx, strValue, dataField.typeId));
            }

            if (mTimestampFieldIndex.has_value()) {
                tuple.timestamp = tuple.at(*mTimestampFieldIndex);
            } else {
                const auto nowFn = module->getOrInsertFunction("rtNow", llvm::FunctionType::get(idTy, {}, false));
                tuple.timestamp = segmentCtx.builder.CreateCall(nowFn, {});
            }

            consumers.front()(segmentCtx, tuple);
            segmentCtx.builder.CreateBr(loopBB);

            segmentCtx.builder.SetInsertPoint(exitBB);
        }

    private:
        io::IOSourceId mIOSourceId;
        core::logical::port::Port mDataSchema;
        std::optional<std::size_t> mTimestampFieldIndex;
        std::chrono::milliseconds mRowDelay;

        static llvm::Value *codegenField(core::physical::segment::SegmentContext &segmentCtx, llvm::Value *stringValue,
                                         const core::common::identifier::TypeId typeId) {
            if (typeId == std::to_underlying(core::common::identifier::Type::STRING)) {
                return stringValue;
            }

            llvm::LLVMContext &ctx = segmentCtx.builder.getContext();
            llvm::Module *module = segmentCtx.builder.GetInsertBlock()->getModule();
            auto *ptrTy = llvm::PointerType::getUnqual(ctx);

            std::string fnName;
            llvm::Type *retTy = segmentCtx.typeRegistry.resolveType(typeId, ctx);

            switch (typeId) {
                case std::to_underlying(core::common::identifier::Type::BOOL): {
                    fnName = "rtParseBool";
                    break;
                }
                case std::to_underlying(core::common::identifier::Type::INT8): {
                    fnName = "rtParseInt8";
                    break;
                }
                case std::to_underlying(core::common::identifier::Type::INT16): {
                    fnName = "rtParseInt16";
                    break;
                }
                case std::to_underlying(core::common::identifier::Type::INT32): {
                    fnName = "rtParseInt32";
                    break;
                }
                case std::to_underlying(core::common::identifier::Type::INT64): {
                    fnName = "rtParseInt64";
                    break;
                }
                case std::to_underlying(core::common::identifier::Type::FLOAT): {
                    fnName = "rtParseFloat";
                    break;
                }
                case std::to_underlying(core::common::identifier::Type::DOUBLE): {
                    fnName = "rtParseDouble";
                    break;
                }
                default:
                    throw std::invalid_argument(
                        "ReadOperator: cant parse given type with typeid " + std::to_string(typeId));
            }

            auto *fnType = llvm::FunctionType::get(retTy, {ptrTy}, false);
            const auto fn = module->getOrInsertFunction(fnName, fnType);
            return segmentCtx.builder.CreateCall(fn, {stringValue});
        }
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SOURCE_READ_OPERATOR_PHYSICAL_H