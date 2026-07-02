#ifndef SIERRA_ODYPLAN_V2_CORE_PHYSICAL_OPERATOR_INTERNAL_OPERATORABILITY_H
#define SIERRA_ODYPLAN_V2_CORE_PHYSICAL_OPERATOR_INTERNAL_OPERATORABILITY_H

#include <llvm/IR/IRBuilder.h>

#include "core/physical/port/port_tuple.h"

namespace sierra::odyplan::v2::core::physical::opr::internal {
    class IProducer {
    public:
        virtual ~IProducer() = default;
        virtual void produce(llvm::IRBuilder<>& builder, port::PortTupleConsumer consumer) = 0;
    };

    class IConsumer {
    public:
        virtual ~IConsumer() = default;
        virtual void consume(llvm::IRBuilder<>& builder, const port::PortTuple& tuple, port::PortTupleConsumer consumer) = 0;
    };
}  // namespace sierra::odyplan::v2::core::physical::opr::internal

#endif  // SIERRA_ODYPLAN_V2_CORE_PHYSICAL_OPERATOR_INTERNAL_OPERATORABILITY_H