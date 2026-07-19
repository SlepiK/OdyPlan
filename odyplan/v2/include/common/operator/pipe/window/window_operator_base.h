#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_WINDOWOPERATORBASE_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_WINDOWOPERATORBASE_H

#include <llvm/IR/Module.h>

#include "common/window/window_type.h"
#include "core/logical/operator/pipe/pipe_operator.h"
#include "core/physical/operator/pipe/pipe_operator.h"

namespace sierra::odyplan::v2::common::opr::pipe {
    namespace logical {
        class WindowOperatorBase : public core::logical::opr::TypedPipeOperator {
        protected:
            template <typename T>
            static void checkBaseConfig(const window::WindowType windowType, const T windowSize, const T windowSlide) {
                if (windowSize == T{0}) {
                    throw std::invalid_argument("window size cant be 0");
                }

                if (windowType == window::WindowType::TUMBLING && windowSlide != windowSize) {
                    throw std::invalid_argument("window size and slide must be equal for TUMBLING");
                }

                if (windowType == window::WindowType::SLIDING && (windowSlide == T{0} || windowSlide > windowSize)) {
                    throw std::invalid_argument("sliding requires 0 < slide <= size");
                }
            }
        };
    }

    namespace physical {
        class WindowOperatorBase : public core::physical::opr::TypedPipeOperator {
        public:
            WindowOperatorBase(core::physical::port::Port inputPort, core::physical::port::Port outputPort, const std::uint64_t windowHandle)
                : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)}, core::physical::port::Ports{std::move(outputPort)}),
                    mWindowHandle(windowHandle) {
            }


            void consume(core::physical::segment::SegmentContext &segmentCtx, const core::physical::port::PortTuple &tuple, core::physical::port::PortTupleConsumer consumer) final {
                llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
                llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

                auto* voidTy = llvm::Type::getVoidTy(ctx);
                auto* i64Ty = llvm::Type::getInt64Ty(ctx);

                llvm::Value* wHandle = llvm::ConstantInt::get(i64Ty, mWindowHandle);

                const auto beginWindowFn = module->getOrInsertFunction("rtWindowBeginRow", llvm::FunctionType::get(voidTy, { i64Ty }, false));
                segmentCtx.builder.CreateCall(beginWindowFn, { wHandle });

                for (std::size_t i = 0; i < getOutputPorts().at(0).size(); ++i) {
                    codegenPush(segmentCtx, wHandle, tuple.at(i), getOutputPorts().at(0).at(i).typeId, i);
                }

                llvm::Value* timestamp = getTimestampValue(segmentCtx, tuple, i64Ty);

                const auto endWindowFn = module->getOrInsertFunction("rtWindowEndRow", llvm::FunctionType::get(voidTy, { i64Ty, i64Ty }, false));
                segmentCtx.builder.CreateCall(endWindowFn, { wHandle, timestamp });

                const auto computeWindowFn = module->getOrInsertFunction("rtWindowCompute", llvm::FunctionType::get(voidTy, { i64Ty }, false));
                segmentCtx.builder.CreateCall(computeWindowFn, { wHandle });

                llvm::Function* drainFn = getOrCreateDrainFn(segmentCtx, consumer);
                segmentCtx.builder.CreateCall(drainFn, {wHandle});
            }

            void end(core::physical::segment::SegmentContext &segmentCtx, core::physical::port::PortTupleConsumer consumer) final {
                llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
                llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

                auto* voidTy = llvm::Type::getVoidTy(ctx);
                auto* i64Ty = llvm::Type::getInt64Ty(ctx);

                llvm::Value* wHandle = llvm::ConstantInt::get(i64Ty, mWindowHandle);

                const auto flushFn = module->getOrInsertFunction("rtWindowFlush", llvm::FunctionType::get(voidTy, { i64Ty }, false));
                segmentCtx.builder.CreateCall(flushFn, { wHandle });

                llvm::Function* drainFn = getOrCreateDrainFn(segmentCtx, consumer);
                segmentCtx.builder.CreateCall(drainFn, {wHandle});
            }

        protected:

            virtual llvm::Value* getTimestampValue(core::physical::segment::SegmentContext& segmentCtx, const core::physical::port::PortTuple& tuple, llvm::Type* i64Ty) = 0;

        private:
            std::uint64_t mWindowHandle;
            llvm::Function* mDrainFn = nullptr;

            llvm::Function* getOrCreateDrainFn(const core::physical::segment::SegmentContext& segmentCtx, core::physical::port::PortTupleConsumer& consumer) {
                if (mDrainFn != nullptr) {
                    return mDrainFn;
                }

                llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
                llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

                auto* voidTy = llvm::Type::getVoidTy(ctx);
                auto* i64Ty = llvm::Type::getInt64Ty(ctx);

                auto* drainFn = llvm::Function::Create(llvm::FunctionType::get(voidTy, { i64Ty }, false), llvm::Function::InternalLinkage, "ody_window_drain_" + std::to_string(mWindowHandle), module);

                drainFn->getArg(0)->setName("windowHandle");

                auto* entryBB = llvm::BasicBlock::Create(ctx, "drainEntry", drainFn);

                core::physical::segment::SegmentContext drainCtx(drainFn, entryBB, ctx, segmentCtx.typeRegistry);

                llvm::Value* wHandle = drainFn->getArg(0);
                drainWindows(drainCtx, wHandle, consumer);

                drainCtx.builder.CreateRetVoid();

                mDrainFn = drainFn;

                return drainFn;
            }


            void drainWindows(core::physical::segment::SegmentContext& segmentCtx, llvm::Value* windowHandle, core::physical::port::PortTupleConsumer& consumer) const {
                llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
                llvm::Function* fn = segmentCtx.builder.GetInsertBlock()->getParent();
                llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

                auto* i64Ty = llvm::Type::getInt64Ty(ctx);

                const auto windowCountFn = module->getOrInsertFunction("rtWindowGetCount", llvm::FunctionType::get(i64Ty, { i64Ty }, false));
                llvm::Value* wCount = segmentCtx.builder.CreateCall(windowCountFn, { windowHandle });

                auto* windowIndex = segmentCtx.storage.get(std::to_underlying(core::common::identifier::Type::INT64));
                segmentCtx.builder.CreateStore(llvm::ConstantInt::get(i64Ty, 0), windowIndex);

                auto* wHeaderBB = llvm::BasicBlock::Create(ctx, "windowHeader", fn);
                auto* wBodyBB = llvm::BasicBlock::Create(ctx, "windowBody", fn);
                auto* wExitBB = llvm::BasicBlock::Create(ctx, "windowExit", fn);

                segmentCtx.builder.CreateBr(wHeaderBB);
                segmentCtx.builder.SetInsertPoint(wHeaderBB);

                llvm::Value* wIndex = segmentCtx.builder.CreateLoad(i64Ty, windowIndex);
                llvm::Value* wCond = segmentCtx.builder.CreateICmpSLT(wIndex, wCount);

                segmentCtx.builder.CreateCondBr(wCond, wBodyBB, wExitBB);

                segmentCtx.builder.SetInsertPoint(wBodyBB);

                const auto windowRowCountFn = module->getOrInsertFunction("rtWindowGetRowCountInWindow",llvm::FunctionType::get(i64Ty, { i64Ty, i64Ty }, false));
                llvm::Value* wRowCount = segmentCtx.builder.CreateCall(windowRowCountFn, { windowHandle, wIndex });

                auto* windowRowIndex = segmentCtx.storage.get(std::to_underlying(core::common::identifier::Type::INT64));
                segmentCtx.builder.CreateStore(llvm::ConstantInt::get(i64Ty, 0), windowRowIndex);

                auto* wRowHeaderBB = llvm::BasicBlock::Create(ctx, "windowRowHeader", fn);
                auto* wRowBodyBB = llvm::BasicBlock::Create(ctx, "windowRowBody", fn);
                auto* wRowExitBB = llvm::BasicBlock::Create(ctx, "windowRowExit", fn);

                segmentCtx.builder.CreateBr(wRowHeaderBB);
                segmentCtx.builder.SetInsertPoint(wRowHeaderBB);

                llvm::Value* wRowIndex = segmentCtx.builder.CreateLoad(i64Ty, windowRowIndex);
                llvm::Value* wRowCond = segmentCtx.builder.CreateICmpSLT(wRowIndex, wRowCount);

                segmentCtx.builder.CreateCondBr(wRowCond, wRowBodyBB, wRowExitBB);

                segmentCtx.builder.SetInsertPoint(wRowBodyBB);

                core::physical::port::PortTuple emit;

                emit.reserve(getOutputPorts().at(0).size());
                for (std::size_t i = 0; i < getOutputPorts().at(0).size(); ++i) {
                    emit.push_back(codegenGet(segmentCtx, windowHandle, wIndex, wRowIndex, llvm::ConstantInt::get(i64Ty, i), getOutputPorts().at(0).at(i).typeId));
                }

                consumer(segmentCtx, emit);

                llvm::Value* next = segmentCtx.builder.CreateAdd(wRowIndex, llvm::ConstantInt::get(i64Ty, 1));
                segmentCtx.builder.CreateStore(next, windowRowIndex);
                segmentCtx.builder.CreateBr(wRowHeaderBB);

                segmentCtx.builder.SetInsertPoint(wRowExitBB);
                llvm::Value* wNext = segmentCtx.builder.CreateAdd(wIndex, llvm::ConstantInt::get(i64Ty, 1));
                segmentCtx.builder.CreateStore(wNext, windowIndex);

                segmentCtx.builder.CreateBr(wHeaderBB);

                segmentCtx.builder.SetInsertPoint(wExitBB);
            }

            static void codegenPush(core::physical::segment::SegmentContext& segmentCtx, llvm::Value* windowHandle, llvm::Value* value, const core::common::identifier::TypeId typeId, const std::size_t fieldIndex) {
                llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
                llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

                auto* voidTy = llvm::Type::getVoidTy(ctx);
                auto* i64Ty = llvm::Type::getInt64Ty(ctx);
                auto* ptrTy = llvm::PointerType::getUnqual(ctx);

                llvm::Value* slot = segmentCtx.storage.get(typeId);
                segmentCtx.builder.CreateStore(value, slot);

                const auto fn = module->getOrInsertFunction("rtWindowPush", llvm::FunctionType::get(voidTy, {i64Ty, i64Ty, ptrTy }, false));
                segmentCtx.builder.CreateCall(fn, {windowHandle, llvm::ConstantInt::get(i64Ty, fieldIndex), slot});
            }

            static llvm::Value* codegenGet(core::physical::segment::SegmentContext& segmentCtx, llvm::Value* windowHandle, llvm::Value* windowIndex, llvm::Value* rowIndex, llvm::Value* dataIndex, core::common::identifier::TypeId typeId) {
                llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
                llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

                auto* i64Ty = llvm::Type::getInt64Ty(ctx);
                auto* ptrTy = llvm::PointerType::getUnqual(ctx);

                const auto fn = module->getOrInsertFunction("rtWindowGetData", llvm::FunctionType::get(ptrTy, {i64Ty, i64Ty, i64Ty, i64Ty}, false));
                llvm::Value* ret = segmentCtx.builder.CreateCall(fn, { windowHandle, windowIndex, rowIndex, dataIndex });

                llvm::Type* dataTy = segmentCtx.typeRegistry.resolveType(typeId, ctx);
                return segmentCtx.builder.CreateLoad(dataTy, ret);
            }

        };
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_WINDOWOPERATORBASE_H