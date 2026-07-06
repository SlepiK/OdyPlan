#ifndef SIERRA_ODYPLAN_V2_EXAMPLE_PROJECT_CONSTINTSOURCEOPERATOR_H
#define SIERRA_ODYPLAN_V2_EXAMPLE_PROJECT_CONSTINTSOURCEOPERATOR_H

#include "core/common/type/type.h"
#include "core/logical/operator/source/source_operator.h"
#include "core/physical/operator/source/source_operator.h"

namespace sierra::odyplan::v2::example::project {

    inline core::logical::port::Port createIntPort() {
        return {
            core::logical::port::PortDescriptor {.typeId= core::common::type::INT,.typeJsonName="int"},
            core::logical::port::PortDescriptor {.typeId= core::common::type::STRING,.typeJsonName="string"},
        };
    }

    namespace logical {
        class ConstantSourceOperator final : public core::logical::opr::TypedSourceOperator {
        public:
            static constexpr sierra::odyplan::v2::core::common::identifier::OperatorTypeId TYPE_ID = 0;

            explicit ConstantSourceOperator() : TypedSourceOperator(std::move(createIntPort())) {
            }

            [[nodiscard]] sierra::odyplan::v2::core::common::identifier::OperatorTypeId getTypeId() const override {
                return TYPE_ID;
            }
        };
    }

    namespace physical {
        class ConstantSourceOperator final : public core::physical::opr::TypedSourceOperator {
        public:
            explicit ConstantSourceOperator(core::physical::port::Port outputPort) : TypedSourceOperator(std::move(outputPort)) {
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

                std::string const exampleString = "ProjectOperator_Test";
                llvm::GlobalVariable* stringRaw = builder.CreateGlobalString(exampleString);
                llvm::StructType* stringType = llvm::StructType::get(builder.getContext(), {
                    llvm::PointerType::get(builder.getContext(), 0),
                    llvm::Type::getInt64Ty(builder.getContext())
                });

                llvm::Value* stringLength = llvm::ConstantInt::get(llvm::Type::getInt64Ty(builder.getContext()), exampleString.size());

                llvm::Value* valueField2 = llvm::PoisonValue::get(stringType);
                valueField2 = builder.CreateInsertValue(valueField2, stringRaw, {0});
                valueField2 = builder.CreateInsertValue(valueField2, stringLength, {1});



                consumer(builder, {valueField1, valueField2});
#ifdef ENABLE_LOOPED_SOURCE
                builder.CreateBr(loopBB);
                builder.SetInsertPoint(exitBB);
#endif
            }
        };
    }

}

#endif //SIERRA_ODYPLAN_V2_EXAMPLE_PROJECT_CONSTINTSOURCEOPERATOR_H