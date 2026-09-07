#ifndef SIERRA_ODYPLAN_V3_EXAMPLE_AOT_SELECT_CONSTINTSOURCEOPERATOR_H
#define SIERRA_ODYPLAN_V3_EXAMPLE_AOT_SELECT_CONSTINTSOURCEOPERATOR_H

#include <core/logical/operators/source/typed_source_operator.h>
#include <core/physical/operators/source/typed_source_operator.h>
#include <core/shared/types/value_type_id.h>

namespace sierra::odyplan::v3::example::aot::select {
    constexpr core::shared::types::ValueTypeId TYPE_INT = 1;

    inline core::logical::port::Port createIntPort() {
        return {core::logical::port::PortDescription{
            .valueTypeId = std::to_underlying(core::shared::types::BuiltinValueType::INT32),
            .typeJsonName = "int"}};
    }

    namespace logical {
        class ConstantIntSourceOperator final
            : public core::logical::operators::source::TypedSourceOperator {
        public:
            static constexpr core::shared::types::OperatorTypeId TYPE_ID = 0;

            explicit ConstantIntSourceOperator()
                : TypedSourceOperator(std::move(createIntPort())) {}

            [[nodiscard]] core::shared::types::OperatorTypeId getOperatorTypeId() const override {
                return TYPE_ID;
            }
        };
    }  // namespace logical

    namespace physical {
        class ConstantIntSourceOperator final
            : public core::physical::operators::source::TypedSourceOperator {
        public:
            explicit ConstantIntSourceOperator(core::physical::port::Port outputPort)
                : TypedSourceOperator(std::move(outputPort)) {}

            void produce(core::physical::segment::SegmentContext& segmentCtx,
                         const core::physical::segment::SegmentDataConsumers& consumers) override {
                llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
                llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();
#ifdef ENABLE_LOOPED_SOURCE
                llvm::Function* fn = segmentCtx.builder.GetInsertBlock()->getParent();

                auto* loopBB = llvm::BasicBlock::Create(ctx, "loop", fn);
                auto* produceBB = llvm::BasicBlock::Create(ctx, "produce", fn);
                auto* exitBB = llvm::BasicBlock::Create(ctx, "exit", fn);

                segmentCtx.builder.CreateBr(loopBB);
                segmentCtx.builder.SetInsertPoint(loopBB);

                auto* stopFnType = llvm::FunctionType::get(llvm::Type::getInt1Ty(ctx), {}, false);

                llvm::FunctionCallee stopFn =
                    segmentCtx.builder.GetInsertBlock()->getModule()->getOrInsertFunction(
                        "odyStop", stopFnType);
                llvm::Value* stop = segmentCtx.builder.CreateCall(stopFn, {});
                segmentCtx.builder.CreateCondBr(stop, exitBB, produceBB);
                segmentCtx.builder.SetInsertPoint(produceBB);
#endif
                llvm::Value* value = llvm::ConstantInt::get(
                    llvm::Type::getInt32Ty(segmentCtx.builder.getContext()), 5);

                core::physical::data::DataPackage tuple(core::physical::time::TimeInterval(segmentCtx.builder, core::physical::time::TimePoint::systemTime(segmentCtx.builder)));
                tuple.reserve(getOutputPorts().size());

                tuple.pushBack(value);

                consumers.front().consume(segmentCtx, tuple);
#ifdef ENABLE_LOOPED_SOURCE
                builder.CreateBr(loopBB);
                builder.SetInsertPoint(exitBB);
#endif

                const core::physical::data::PunctuationPackage finalPunctuation{
                    .watermark = llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx), 0),
                    .isFinal = llvm::ConstantInt::getTrue(ctx)
                };
                consumers.front().consumePunctuation(segmentCtx, finalPunctuation);
            }
        };
    }  // namespace physical

}  // namespace sierra::odyplan::v3::example::aot::select

#endif  // SIERRA_ODYPLAN_V3_EXAMPLE_AOT_SELECT_CONSTINTSOURCEOPERATOR_H