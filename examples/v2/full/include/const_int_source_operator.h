#ifndef SIERRA_ODYPLAN_V2_EXAMPLE_FULL_CONSTINTSOURCEOPERATOR_H
#define SIERRA_ODYPLAN_V2_EXAMPLE_FULL_CONSTINTSOURCEOPERATOR_H

#include "core/logical/operator/source/source_operator.h"
#include "core/physical/operator/source/source_operator.h"

namespace sierra::odyplan::v2::example::full {
    constexpr core::common::identifier::PortTypeId TYPE_INT = 1;

    inline core::logical::port::Port createIntPort() {
        return { core::logical::port::PortDescriptor {.typeId=TYPE_INT,.typeJsonName="int32"} };
    }

    namespace logical {
        class ConstantIntSourceOperator final : public core::logical::opr::TypedSourceOperator {
        public:
            static constexpr sierra::odyplan::v2::core::common::identifier::OperatorTypeId TYPE_ID = 0;

            explicit ConstantIntSourceOperator() : TypedSourceOperator(std::move(createIntPort())) {
            }

            [[nodiscard]] sierra::odyplan::v2::core::common::identifier::OperatorTypeId getTypeId() const override {
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
                llvm::Value* value = llvm::ConstantInt::get(llvm::Type::getInt32Ty(builder.getContext()),5);

                consumer(builder, {value});
            }
        };
    }

}

#endif //SIERRA_ODYPLAN_V2_EXAMPLE_FULL_CONSTINTSOURCEOPERATOR_H