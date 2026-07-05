#ifndef SIERRA_ODYPLAN_V2_EXAMPLE_PROJECT_PRINTINTSINKOPERATOR_H
#define SIERRA_ODYPLAN_V2_EXAMPLE_PROJECT_PRINTINTSINKOPERATOR_H

#include <llvm/IR/Module.h>

#include "core/logical/operator/sink/sink_operator.h"
#include "core/physical/operator/sink/sink_operator.h"
#include "const_int_source_operator.h"

namespace sierra::odyplan::v2::example::project {
    namespace logical {
        class PrintIntSinkOperator final : public core::logical::opr::TypedSinkOperator {
        public:
            static constexpr sierra::odyplan::v2::core::common::identifier::OperatorTypeId TYPE_ID = 1;

            explicit PrintIntSinkOperator() : TypedSinkOperator(createProjectTestPort()) {
            }

            [[nodiscard]] sierra::odyplan::v2::core::common::identifier::OperatorTypeId getTypeId() const override {
                return TYPE_ID;
            }
        };
    }

    namespace physical {
        class PrintIntSinkOperator final : public core::physical::opr::TypedSinkOperator {
        public:
            explicit PrintIntSinkOperator(core::physical::port::Port outputPort)
                : TypedSinkOperator(std::move(outputPort)) {
            }

            void consume(llvm::IRBuilder<>& builder, const core::physical::port::PortTuple& tuple, core::physical::port::PortTupleConsumer consumer) override {
                llvm::Module* module = builder.GetInsertBlock()->getModule();
                llvm::LLVMContext& ctx = builder.getContext();

                auto* functionType = llvm::FunctionType::get(
                    llvm::Type::getVoidTy(ctx),
                    {llvm::Type::getInt32Ty(ctx)},
                    false);

                llvm::FunctionCallee callee = module->getOrInsertFunction("odyExamplePrint", functionType);

                llvm::Value* valueInt = tuple.at(0);
                llvm::Value* valueString = tuple.at(1);

                llvm::Value* stringRaw = builder.CreateExtractValue(valueString,{0});
                llvm::Value* stringLength = builder.CreateExtractValue(valueString,{1});

                builder.CreateCall(callee, {valueInt, stringRaw,stringLength});
            }
        };
    }

}

#endif //SIERRA_ODYPLAN_V2_EXAMPLE_PROJECT_PRINTINTSINKOPERATOR_H