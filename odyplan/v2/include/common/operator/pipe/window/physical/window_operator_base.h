#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_BASEOPERATOR_PHYSICAL_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_BASEOPERATOR_PHYSICAL_H

#include <llvm/IR/Module.h>

#include "core/physical/operator/pipe/pipe_operator.h"
#include "core/logical/port/port.h"

namespace sierra::odyplan::v2::common::opr::pipe::physical {
    class WindowOperatorBase : public core::physical::opr::TypedPipeOperator {
    public:
        WindowOperatorBase(core::physical::port::Port inputPort, core::physical::port::Port outputPort)
            : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)},
                                core::physical::port::Ports{std::move(outputPort)}) {
        }


        void consume(core::physical::segment::SegmentContext &segmentCtx,
                     core::common::identifier::PortId /*inputPort*/, const core::physical::port::PortTuple &tuple,
                     const core::physical::port::PortTupleConsumers &consumers) final {
            llvm::LLVMContext &ctx = segmentCtx.builder.getContext();
            llvm::Module *module = segmentCtx.builder.GetInsertBlock()->getModule();

            auto *voidTy = llvm::Type::getVoidTy(ctx);
            auto *i64Ty = llvm::Type::getInt64Ty(ctx);
            auto *ptrTy = llvm::PointerType::getUnqual(ctx);

            llvm::Value* state = loadStatePtr(segmentCtx);

            const auto beginWindowFn = module->getOrInsertFunction("rtWindowBeginRow",
                                                                   llvm::FunctionType::get(voidTy, {ptrTy}, false));
            segmentCtx.builder.CreateCall(beginWindowFn, {state});

            for (std::size_t i = 0; i < getOutputPorts().at(0).size(); ++i) {
                codegenPush(segmentCtx, state, tuple.at(i), getOutputPorts().at(0).at(i).typeId, i);
            }

            llvm::Value* timestamp = tuple.timestamp;

            const auto endWindowFn = module->getOrInsertFunction("rtWindowEndRow",
                                                                 llvm::FunctionType::get(voidTy, {ptrTy, i64Ty}, false));

            segmentCtx.builder.CreateCall(endWindowFn, {state, timestamp});

            const auto computeWindowFn = module->getOrInsertFunction("rtWindowCompute",
                                                                     llvm::FunctionType::get(voidTy, {ptrTy}, false));
            segmentCtx.builder.CreateCall(computeWindowFn, {state});

            llvm::Function *drainFn = getOrCreateDrainFn(segmentCtx, consumers.front());
            segmentCtx.builder.CreateCall(drainFn, {state});
        }

        void end(core::physical::segment::SegmentContext &segmentCtx,
                 const core::common::identifier::PortId /*inputPort*/,
                 const core::physical::port::PortTupleConsumers &consumers) final {
            llvm::LLVMContext &ctx = segmentCtx.builder.getContext();
            llvm::Module *module = segmentCtx.builder.GetInsertBlock()->getModule();

            auto *voidTy = llvm::Type::getVoidTy(ctx);
            auto *ptrTy = llvm::PointerType::getUnqual(ctx);

            llvm::Value *state = loadStatePtr(segmentCtx);

            const auto flushFn = module->getOrInsertFunction("rtWindowFlush",
                                                             llvm::FunctionType::get(voidTy, {ptrTy}, false));
            segmentCtx.builder.CreateCall(flushFn, {state});

            llvm::Function *drainFn = getOrCreateDrainFn(segmentCtx, consumers.front());
            segmentCtx.builder.CreateCall(drainFn, {state});
        }

    protected:
        void setWindowState(const core::physical::state::StateHandle state) {
            mState = state;
        }

        static llvm::GlobalVariable* buildTypeIds(llvm::Module& module, llvm::LLVMContext& ctx,
                                                    const core::logical::port::Port& schema, const std::string& name) {
            auto* i8Ty = llvm::Type::getInt8Ty(ctx);

            std::vector<llvm::Constant*> values;
            values.reserve(schema.size());

            for (const auto& field : schema) {
                values.push_back(llvm::ConstantInt::get(i8Ty, field.typeId));
            }

            auto* arrTy = llvm::ArrayType::get(i8Ty, values.size());

            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,clang-analyzer-cplusplus.NewDeleteLeaks)
            return new llvm::GlobalVariable(module, arrTy, true, llvm::GlobalValue::InternalLinkage,
                                             llvm::ConstantArray::get(arrTy, values), name);
        }

    private:
        core::physical::state::StateHandle mState;
        llvm::Function *mDrainFn = nullptr;

        llvm::Value* loadStatePtr(core::physical::segment::SegmentContext &segmentCtx) const {
            auto *ptrTy = llvm::PointerType::getUnqual(segmentCtx.builder.getContext());
            return segmentCtx.builder.CreateLoad(ptrTy, mState.var);
        }

        llvm::Function *getOrCreateDrainFn(const core::physical::segment::SegmentContext &segmentCtx,
                                           const core::physical::port::PortTupleConsumer &consumer) {
            if (mDrainFn != nullptr) {
                return mDrainFn;
            }

            llvm::LLVMContext &ctx = segmentCtx.builder.getContext();
            llvm::Module *module = segmentCtx.builder.GetInsertBlock()->getModule();

            auto *voidTy = llvm::Type::getVoidTy(ctx);
            auto *ptrTy = llvm::PointerType::getUnqual(ctx);

            auto *drainFn = llvm::Function::Create(llvm::FunctionType::get(voidTy, {ptrTy}, false),
                                                   llvm::Function::InternalLinkage,
                                                   "ody_window_drain_" + std::to_string(getId()), module);

            drainFn->getArg(0)->setName("windowHandle");

            auto *entryBB = llvm::BasicBlock::Create(ctx, "drainEntry", drainFn);

            core::physical::segment::SegmentContext drainCtx(drainFn, entryBB, ctx, segmentCtx.typeRegistry);

            llvm::Value *state = drainFn->getArg(0);
            drainWindows(drainCtx, state, consumer);

            drainCtx.builder.CreateRetVoid();

            mDrainFn = drainFn;

            return drainFn;
        }


        void drainWindows(core::physical::segment::SegmentContext &segmentCtx, llvm::Value *state,
                          const core::physical::port::PortTupleConsumer &consumer) const {
            llvm::LLVMContext &ctx = segmentCtx.builder.getContext();
            llvm::Function *fn = segmentCtx.builder.GetInsertBlock()->getParent();
            llvm::Module *module = segmentCtx.builder.GetInsertBlock()->getModule();

            auto *i64Ty = llvm::Type::getInt64Ty(ctx);
            auto *ptrTy = llvm::PointerType::getUnqual(ctx);

            const auto windowCountFn = module->getOrInsertFunction("rtWindowGetCount",
                                                                   llvm::FunctionType::get(i64Ty, {ptrTy}, false));
            llvm::Value *wCount = segmentCtx.builder.CreateCall(windowCountFn, {state});

            auto *windowIndex = segmentCtx.storage.get(std::to_underlying(core::common::identifier::Type::INT64));
            segmentCtx.builder.CreateStore(llvm::ConstantInt::get(i64Ty, 0), windowIndex);

            auto *wHeaderBB = llvm::BasicBlock::Create(ctx, "windowHeader", fn);
            auto *wBodyBB = llvm::BasicBlock::Create(ctx, "windowBody", fn);
            auto *wExitBB = llvm::BasicBlock::Create(ctx, "windowExit", fn);

            segmentCtx.builder.CreateBr(wHeaderBB);
            segmentCtx.builder.SetInsertPoint(wHeaderBB);

            llvm::Value *wIndex = segmentCtx.builder.CreateLoad(i64Ty, windowIndex);
            llvm::Value *wCond = segmentCtx.builder.CreateICmpSLT(wIndex, wCount);

            segmentCtx.builder.CreateCondBr(wCond, wBodyBB, wExitBB);

            segmentCtx.builder.SetInsertPoint(wBodyBB);

            const auto windowRowCountFn = module->getOrInsertFunction("rtWindowGetRowCountInWindow",
                                                                      llvm::FunctionType::get(
                                                                          i64Ty, {ptrTy, i64Ty}, false));
            llvm::Value *wRowCount = segmentCtx.builder.CreateCall(windowRowCountFn, {state, wIndex});

            auto *windowRowIndex = segmentCtx.storage.get(std::to_underlying(core::common::identifier::Type::INT64));
            segmentCtx.builder.CreateStore(llvm::ConstantInt::get(i64Ty, 0), windowRowIndex);

            auto *wRowHeaderBB = llvm::BasicBlock::Create(ctx, "windowRowHeader", fn);
            auto *wRowBodyBB = llvm::BasicBlock::Create(ctx, "windowRowBody", fn);
            auto *wRowExitBB = llvm::BasicBlock::Create(ctx, "windowRowExit", fn);

            segmentCtx.builder.CreateBr(wRowHeaderBB);
            segmentCtx.builder.SetInsertPoint(wRowHeaderBB);

            llvm::Value *wRowIndex = segmentCtx.builder.CreateLoad(i64Ty, windowRowIndex);
            llvm::Value *wRowCond = segmentCtx.builder.CreateICmpSLT(wRowIndex, wRowCount);

            segmentCtx.builder.CreateCondBr(wRowCond, wRowBodyBB, wRowExitBB);

            segmentCtx.builder.SetInsertPoint(wRowBodyBB);

            core::physical::port::PortTuple emit;

            emit.timestamp = codegenGetTimestamp(segmentCtx, state, wIndex, wRowIndex);

            emit.reserve(getOutputPorts().at(0).size());
            for (std::size_t i = 0; i < getOutputPorts().at(0).size(); ++i) {
                emit.pushBack(codegenGet(segmentCtx, state, wIndex, wRowIndex, llvm::ConstantInt::get(i64Ty, i),
                                          getOutputPorts().at(0).at(i).typeId));
            }

            consumer(segmentCtx, emit);

            llvm::Value *next = segmentCtx.builder.CreateAdd(wRowIndex, llvm::ConstantInt::get(i64Ty, 1));
            segmentCtx.builder.CreateStore(next, windowRowIndex);
            segmentCtx.builder.CreateBr(wRowHeaderBB);

            segmentCtx.builder.SetInsertPoint(wRowExitBB);
            llvm::Value *wNext = segmentCtx.builder.CreateAdd(wIndex, llvm::ConstantInt::get(i64Ty, 1));
            segmentCtx.builder.CreateStore(wNext, windowIndex);

            segmentCtx.builder.CreateBr(wHeaderBB);

            segmentCtx.builder.SetInsertPoint(wExitBB);
        }

        static llvm::Value* codegenGetTimestamp(core::physical::segment::SegmentContext& segmentContext, llvm::Value* state, llvm::Value* windowIndex, llvm::Value* rowIndex) {
            llvm::LLVMContext& ctx = segmentContext.builder.getContext();
            llvm::Module* module = segmentContext.builder.GetInsertBlock()->getModule();

            auto* i64Ty = llvm::Type::getInt64Ty(ctx);
            auto* ptrTy = llvm::PointerType::getUnqual(ctx);

            const auto fn = module->getOrInsertFunction("rtWindowGetTimestamp",llvm::FunctionType::get(i64Ty, {ptrTy, i64Ty, i64Ty}, false));
            return segmentContext.builder.CreateCall(fn, {state, windowIndex, rowIndex});
        }

        static void codegenPush(core::physical::segment::SegmentContext &segmentCtx, llvm::Value *state,
                                llvm::Value *value, const core::common::identifier::TypeId typeId,
                                const std::size_t fieldIndex) {
            llvm::LLVMContext &ctx = segmentCtx.builder.getContext();
            llvm::Module *module = segmentCtx.builder.GetInsertBlock()->getModule();

            auto *voidTy = llvm::Type::getVoidTy(ctx);
            auto *i64Ty = llvm::Type::getInt64Ty(ctx);
            auto *ptrTy = llvm::PointerType::getUnqual(ctx);

            llvm::Value *slot = segmentCtx.storage.get(typeId);
            segmentCtx.builder.CreateStore(value, slot);

            const auto fn = module->getOrInsertFunction("rtWindowPush",
                                                        llvm::FunctionType::get(voidTy, {ptrTy, i64Ty, ptrTy}, false));
            segmentCtx.builder.CreateCall(fn, {state, llvm::ConstantInt::get(i64Ty, fieldIndex), slot});
        }

        static llvm::Value* codegenGet(core::physical::segment::SegmentContext &segmentCtx, llvm::Value *state,
                                       llvm::Value *windowIndex, llvm::Value *rowIndex, llvm::Value *dataIndex,
                                       const core::common::identifier::TypeId typeId) {
            llvm::LLVMContext &ctx = segmentCtx.builder.getContext();
            llvm::Module *module = segmentCtx.builder.GetInsertBlock()->getModule();

            auto *i64Ty = llvm::Type::getInt64Ty(ctx);
            auto *ptrTy = llvm::PointerType::getUnqual(ctx);

            const auto fn = module->getOrInsertFunction("rtWindowGetData",
                                                        llvm::FunctionType::get(
                                                            ptrTy, {ptrTy, i64Ty, i64Ty, i64Ty}, false));
            llvm::Value *ret = segmentCtx.builder.CreateCall(fn, {state, windowIndex, rowIndex, dataIndex});

            llvm::Type *dataTy = segmentCtx.typeRegistry.resolveType(typeId, ctx);
            return segmentCtx.builder.CreateLoad(dataTy, ret);
        }
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_BASEOPERATOR_PHYSICAL_H