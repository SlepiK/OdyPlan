#ifndef SIERRA_ODYPLAN_V2_EXAMPLE_PROJECT_CONSTINTSOURCEOPERATOR_H
#define SIERRA_ODYPLAN_V2_EXAMPLE_PROJECT_CONSTINTSOURCEOPERATOR_H

#include "core/logical/operator/source/source_operator.h"
#include "core/physical/operator/source/source_operator.h"

namespace sierra::odyplan::v2::example::project {
    constexpr core::common::identifier::PortTypeId TYPE_INT = 1;
    constexpr core::common::identifier::PortTypeId TYPE_STRING = 2;

    inline core::logical::port::Port createProjectTestPort() {
        return {
            core::logical::port::PortDescriptor {.typeId=TYPE_INT,.typeJsonName="int32",.fieldName="field_1"},
            core::logical::port::PortDescriptor {.typeId=TYPE_STRING,.typeJsonName="string",.fieldName="field_2"},
            core::logical::port::PortDescriptor {.typeId=TYPE_INT,.typeJsonName="int32",.fieldName="field_3"}
        };
    }

    namespace logical {
        class ConstantIntSourceOperator final : public core::logical::opr::TypedSourceOperator {
        public:
            static constexpr core::common::identifier::OperatorTypeId TYPE_ID = 0;

            explicit ConstantIntSourceOperator() : TypedSourceOperator(std::move(createProjectTestPort())) {
            }

            [[nodiscard]] core::common::identifier::OperatorTypeId getTypeId() const override {
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
                llvm::Value* valueField1 = llvm::ConstantInt::get(llvm::Type::getInt32Ty(builder.getContext()),5);

                std::string const literal = "ProjectOperator_Test";
                llvm::GlobalVariable* stringRaw = builder.CreateGlobalString(literal);
                llvm::StructType* stringType = llvm::StructType::get(builder.getContext(), {
                    llvm::PointerType::get(builder.getContext(), 0),
                    llvm::Type::getInt64Ty(builder.getContext())
                });

                llvm::Value* stringLength = llvm::ConstantInt::get(llvm::Type::getInt32Ty(builder.getContext()),literal.size());

                llvm::Value* valueField2 = llvm::PoisonValue::get(stringType);
                valueField2 = builder.CreateInsertValue(valueField2, stringRaw, {0});
                valueField2 = builder.CreateInsertValue(valueField2, stringLength, {1});

                llvm::Value* valueField3 = llvm::ConstantInt::get(llvm::Type::getInt32Ty(builder.getContext()),15);

                consumer(builder, {valueField1,valueField2,valueField3});
#ifdef ENABLE_LOOPED_SOURCE
                builder.CreateBr(loopBB);
                builder.SetInsertPoint(exitBB);
#endif
            }
        };
    }

}

#endif //SIERRA_ODYPLAN_V2_EXAMPLE_PROJECT_CONSTINTSOURCEOPERATOR_H