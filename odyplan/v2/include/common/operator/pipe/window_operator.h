#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOWOPERATOR_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOWOPERATOR_H

#include <llvm/IR/Module.h>

#include "common/window/window_config.h"
#include "core/logical/operator/pipe/pipe_operator.h"
#include "core/physical/operator/pipe/pipe_operator.h"

namespace sierra::odyplan::v2::common::opr::pipe {
    namespace logical {
        class WindowOperator : public core::logical::opr::TypedPipeOperator {
        public:
            enum class WindowType : std::uint8_t {
                TUMBLING,
                SLIDING,
            };

            static constexpr core::common::identifier::OperatorTypeId TYPE_ID = std::to_underlying(core::common::identifier::Operator::WINDOW);

            explicit WindowOperator(window::WindowConfig config)
                : mWindowConfig(std::move(config)) {
                std::visit([](const auto& wConfig) {checkConfig(wConfig); }, mWindowConfig);
            }

            [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
                return TYPE_ID;
            }

            [[nodiscard]] const window::WindowConfig& getWindowConfig() const {
                return mWindowConfig;
            }

        protected:
            void validate() override {
            }

        private:
            window::WindowConfig mWindowConfig;

            template <typename T>
            static void checkCommonConfig(const WindowType windowType, const T windowSize, const T windowSlide) {
                if (windowSize == T{0}) {
                    throw std::invalid_argument("window size cant be 0");
                }

                if (windowType == WindowType::TUMBLING && windowSlide != windowSize) {
                    throw std::invalid_argument("window size and slide must be equal for TUMBLING");
                }

                if (windowType == WindowType::SLIDING && (windowSlide == T{0} || windowSlide > windowSize)) {
                    throw std::invalid_argument("sliding requires 0 < slide <= size");
                }
            }

            static void checkConfig(const window::CountWindowConfig& config) {
                checkCommonConfig(config.type, config.size, config.slide);
            }

            static void checkConfig(const window::TimeWindowConfig& config) {
                checkCommonConfig(config.type, config.size, config.slide);
            }
        };
    }

    namespace physical {
        class WindowOperator : public core::physical::opr::TypedBlockingPipeOperator {
        public:
            explicit WindowOperator(core::physical::port::Port inputPort, core::physical::port::Port outputPort,
                const std::uint64_t windowHandle)
                : TypedBlockingPipeOperator(core::physical::port::Ports{std::move(inputPort)}, core::physical::port::Ports{std::move(outputPort)}),
                    mWindowHandle(windowHandle) {
            }

        protected:
            //consume
            void build(core::physical::segment::SegmentContext& segmentCtx, const core::physical::port::PortTuple& tuple) override {
                llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
                llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

                auto* voidTy = llvm::Type::getVoidTy(ctx);
                auto* i64Ty = llvm::Type::getInt64Ty(ctx);
                llvm::Value* wHandle = llvm::ConstantInt::get(i64Ty, mWindowHandle);

                auto beginWindowFn = module->getOrInsertFunction("rtWindowBeginRow", llvm::FunctionType::get(voidTy, { i64Ty }, false));
                segmentCtx.builder.CreateCall(beginWindowFn, { wHandle });

                for (std::size_t i = 0; i < getOutputPorts().at(0).size(); ++i) {
                    codegenPush(segmentCtx, wHandle, tuple.at(i), getOutputPorts().at(0).at(i).typeId);
                }

                auto endWindowFn = module->getOrInsertFunction("rtWindowEndRow", llvm::FunctionType::get(voidTy, { i64Ty, i64Ty }, false));
                segmentCtx.builder.CreateCall(endWindowFn, { wHandle, llvm::ConstantInt::get(i64Ty, 0) });
            }
            //produce
            void probe(core::physical::segment::SegmentContext& segmentCtx, core::physical::port::PortTupleConsumer consumer) override {
                llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
                llvm::Function* fn = segmentCtx.builder.GetInsertBlock()->getParent();
                llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

                auto* voidTy = llvm::Type::getVoidTy(ctx);
                auto* i1Ty = llvm::Type::getInt1Ty(ctx);
                auto* i64Ty = llvm::Type::getInt64Ty(ctx);
                auto* ptrTy = llvm::PointerType::getUnqual(ctx);
                llvm::Value* wHandle = llvm::ConstantInt::get(i64Ty, mWindowHandle);

                auto computeWindowFn = module->getOrInsertFunction("rtWindowCompute", llvm::FunctionType::get(voidTy, { i64Ty }, false));
                segmentCtx.builder.CreateCall(computeWindowFn, { wHandle });

                auto windowCountFn = module->getOrInsertFunction("rtWindowCount", llvm::FunctionType::get(i64Ty, { i64Ty }, false));
                llvm::Value* wCount = segmentCtx.builder.CreateCall(windowCountFn, { wHandle });

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

                auto windowRowCountFn = module->getOrInsertFunction("rtWindowRowCount",llvm::FunctionType::get(i64Ty, { i64Ty, i64Ty }, false));
                llvm::Value* wRowCount = segmentCtx.builder.CreateCall(windowRowCountFn, { wHandle, wIndex });

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
                    emit.push_back(codegenGet(segmentCtx, wHandle, wIndex, wRowIndex, llvm::ConstantInt::get(i64Ty, i), getOutputPorts().at(0).at(i).typeId));
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
        private:
            std::uint64_t mWindowHandle;

            void codegenPush(core::physical::segment::SegmentContext& segmentCtx, llvm::Value* windowHandle, llvm::Value* value, const core::common::identifier::TypeId typeId) {
                llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
                llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

                auto* voidTy = llvm::Type::getVoidTy(ctx);
                auto* i64Ty = llvm::Type::getInt64Ty(ctx);
                auto* ptrTy = llvm::PointerType::getUnqual(ctx);

                auto fn = module->getOrInsertFunction("rtWindowPush", llvm::FunctionType::get(voidTy, {i64Ty, i64Ty, ptrTy }, false));
                segmentCtx.builder.CreateCall(fn, {windowHandle, llvm::ConstantInt::get(i64Ty, typeId), value});
            }

            llvm::Value* codegenGet(core::physical::segment::SegmentContext& segmentCtx, llvm::Value* windowHandle, llvm::Value* windowIndex, llvm::Value* rowIndex, llvm::Value* dataIndex, core::common::identifier::TypeId typeId) {
                llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
                llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

                auto* i64Ty = llvm::Type::getInt64Ty(ctx);
                auto* ptrTy = llvm::PointerType::getUnqual(ctx);

                auto fn = module->getOrInsertFunction("rtWindowGet", llvm::FunctionType::get(ptrTy, {i64Ty, i64Ty, i64Ty, i64Ty}, false));
                llvm::Value* ret = segmentCtx.builder.CreateCall(fn, { windowHandle, windowIndex, rowIndex, dataIndex });
                return segmentCtx.builder.CreateLoad(segmentCtx.typeRegistry.resolveType(typeId, ctx), ret);
            }
        };
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOWOPERATOR_H