#ifndef SIERRA_ODYPLAN_V3_EXAMPLE_PROJECT_CONSTINTSOURCEOPERATOR_H
#define SIERRA_ODYPLAN_V3_EXAMPLE_PROJECT_CONSTINTSOURCEOPERATOR_H

#include <core/logical/operators/source/typed_source_operator.h>
#include <core/physical/operators/source/typed_source_operator.h>
#include <core/shared/types/value_type_id.h>

namespace sierra::odyplan::v3::example::aot::project {

    inline core::logical::port::Port createIntPort() {
        return {
            core::logical::port::PortDescription{
                .valueTypeId = std::to_underlying(core::shared::types::BuiltinValueType::INT32),
                .typeJsonName = "int"},
            core::logical::port::PortDescription{
                .valueTypeId = std::to_underlying(core::shared::types::BuiltinValueType::STRING),
                .typeJsonName = "string"},
        };
    }

    namespace logical {
        class ConstantSourceOperator final
            : public core::logical::operators::source::TypedSourceOperator {
        public:
            static constexpr core::shared::types::OperatorTypeId TYPE_ID = 0;

            explicit ConstantSourceOperator() : TypedSourceOperator(std::move(createIntPort())) {}

            [[nodiscard]] core::shared::types::OperatorTypeId getOperatorTypeId() const override {
                return TYPE_ID;
            }
        };
    }  // namespace logical

    namespace physical {
        class ConstantSourceOperator final
            : public core::physical::operators::source::TypedSourceOperator {
        public:
            explicit ConstantSourceOperator(core::physical::port::Port outputPort)
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
                llvm::Value* valueField1 = llvm::ConstantInt::get(
                    llvm::Type::getInt32Ty(segmentCtx.builder.getContext()), 5);

                std::string const exampleString = "ProjectOperator_Test";
                llvm::GlobalVariable* stringRaw =
                    segmentCtx.builder.CreateGlobalString(exampleString);
                llvm::StructType* stringType = llvm::StructType::get(
                    segmentCtx.builder.getContext(),
                    {llvm::PointerType::get(segmentCtx.builder.getContext(), 0),
                     llvm::Type::getInt64Ty(segmentCtx.builder.getContext())});

                llvm::Value* stringLength = llvm::ConstantInt::get(
                    llvm::Type::getInt64Ty(segmentCtx.builder.getContext()), exampleString.size());

                llvm::Value* valueField2 = llvm::PoisonValue::get(stringType);
                valueField2 = segmentCtx.builder.CreateInsertValue(valueField2, stringRaw, {0});
                valueField2 = segmentCtx.builder.CreateInsertValue(valueField2, stringLength, {1});

                core::physical::data::DataPackage tuple;
                tuple.reserve(getOutputPorts().size());

                tuple.pushBack(valueField1);
                tuple.pushBack(valueField2);

                const auto nowFn = module->getOrInsertFunction(
                    "rtNow", llvm::FunctionType::get(llvm::Type::getInt64Ty(ctx), {}, false));
                tuple.timestamp = segmentCtx.builder.CreateCall(nowFn, {});

                consumers.front()(segmentCtx, tuple);
#ifdef ENABLE_LOOPED_SOURCE
                builder.CreateBr(loopBB);
                builder.SetInsertPoint(exitBB);
#endif
            }
        };
    }  // namespace physical

}  // namespace sierra::odyplan::v3::example::aot::project

#endif  // SIERRA_ODYPLAN_V3_EXAMPLE_PROJECT_CONSTINTSOURCEOPERATOR_H