#ifndef SIERRA_ODYPLAN_V2_EXAMPLE_LOGICALPLAN_CONSTNUMBERSOURCEOPERATOR_H
#define SIERRA_ODYPLAN_V2_EXAMPLE_LOGICALPLAN_CONSTNUMBERSOURCEOPERATOR_H

#include "core/logical/operator/source/source_operator.h"

namespace sierra::odyplan::v2::example::logical_plan {
    constexpr core::common::identifier::TypeId TYPE_INT = 1;
    constexpr core::common::identifier::TypeId TYPE_DOUBLE = 2;

    inline core::logical::port::Port createIntPort() {
        return { core::logical::port::PortDescriptor {.typeId=TYPE_INT,.typeJsonName="int"} };
    }

    inline core::logical::port::Port createDoublePort() {
        return { core::logical::port::PortDescriptor {.typeId=TYPE_DOUBLE,.typeJsonName="double"} };
    }

    class ConstNumberSourceOperator final : public core::logical::opr::TypedSourceOperator {
    public:
        static constexpr core::common::identifier::OperatorTypeId TYPE_ID = 0;

        ConstNumberSourceOperator() : core::logical::opr::TypedSourceOperator(createIntPort()) {
        }

        [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
            return TYPE_ID;
        }
    };

    class ConstDoubleNumberSourceOperator final : public core::logical::opr::TypedSourceOperator {
    public:
        static constexpr core::common::identifier::OperatorTypeId TYPE_ID = 3;

        ConstDoubleNumberSourceOperator() : core::logical::opr::TypedSourceOperator(createDoublePort()) {
        }

        [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
            return TYPE_ID;
        }
    };
} // namespace sierra::odyplan::v2::example::logical_plan

#endif //SIERRA_ODYPLAN_V2_EXAMPLE_LOGICALPLAN_CONSTNUMBERSOURCEOPERATOR_H