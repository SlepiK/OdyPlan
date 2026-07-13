#ifndef SIERRA_ODYPLAN_V2_EXAMPLE_SELECT_CONSTINTSOURCEOPERATOR_H
#define SIERRA_ODYPLAN_V2_EXAMPLE_SELECT_CONSTINTSOURCEOPERATOR_H

#include "../../../../odyplan/v2/include/core/common/identifier/type.h"
#include "core/logical/operator/source/source_operator.h"
#include "core/physical/operator/source/source_operator.h"

namespace sierra::odyplan::v2::example::select {
    constexpr core::common::identifier::TypeId TYPE_INT = 1;

    inline core::logical::port::Port createIntPort() {
        return { core::logical::port::PortDescriptor {.typeId= std::to_underlying(core::common::identifier::Type::INT32),.typeJsonName="int"} };
    }

    namespace logical {
        class ConstantIntSourceOperator final : public core::logical::opr::TypedSourceOperator {
        public:
            static constexpr core::common::identifier::OperatorTypeId TYPE_ID = 0;

            explicit ConstantIntSourceOperator() : TypedSourceOperator(std::move(createIntPort())) {
            }

            [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
                return TYPE_ID;
            }
        };
    }

    namespace physical {
        class ConstantIntSourceOperator final : public core::physical::opr::TypedSourceOperator {
        public:
            explicit ConstantIntSourceOperator(core::physical::port::Port outputPort) : TypedSourceOperator(std::move(outputPort)) {
            }

            void produce(llvm::IRBuilder<>& builder, core::physical::port::PortTupleConsumer consumer) override {
#ifdef ENABLE_LOOPED_SOURCE
                llvm::LLVMContext& ctx = builder.getContext();
                llvm::Function* fn = builder.GetInsertBlock()->getParent();

                auto* loopBB = llvm::BasicBlock::Create(ctx, "loop", fn);
                auto* produceBB = llvm::BasicBlock::Create(ctx, "produce", fn);
                auto* exitBB = llvm::BasicBlock::Create(ctx, "exit", fn);

                builder.CreateBr(loopBB);
                builder.SetInsertPoint(loopBB);

                auto* stopFnType = llvm::FunctionType::get(
                    llvm::Type::getInt1Ty(ctx), {}, false);

                llvm::FunctionCallee stopFn = builder.GetInsertBlock()->getModule()->getOrInsertFunction("odyStop", stopFnType);
                llvm::Value* stop = builder.CreateCall(stopFn, {});
                builder.CreateCondBr(stop, exitBB, produceBB);
                builder.SetInsertPoint(produceBB);
#endif
                llvm::Value* value = llvm::ConstantInt::get(llvm::Type::getInt32Ty(builder.getContext()),5);

                consumer(builder, {value});
#ifdef ENABLE_LOOPED_SOURCE
                builder.CreateBr(loopBB);
                builder.SetInsertPoint(exitBB);
#endif
            }
        };
    }

}

#endif //SIERRA_ODYPLAN_V2_EXAMPLE_SELECT_CONSTINTSOURCEOPERATOR_H