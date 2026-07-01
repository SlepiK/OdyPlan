#ifndef SIERRA_ODYPLAN_V2_EXAMPLE_LOGICALPLAN_CONSTNUMBERSOURCEOPERATOR_H
#define SIERRA_ODYPLAN_V2_EXAMPLE_LOGICALPLAN_CONSTNUMBERSOURCEOPERATOR_H

#include "core/logical/operator/source/source_operator.h"

namespace sierra::odyplan::v2::example::logical_plan {
    constexpr sierra::odyplan::v2::core::logical::port::PortTypeId TYPE_INT = 1;
    constexpr sierra::odyplan::v2::core::logical::port::PortTypeId TYPE_DOUBLE = 2;

    inline sierra::odyplan::v2::core::logical::port::Port createIntPort() {
        return { sierra::odyplan::v2::core::logical::port::PortDescriptor {.typeId=TYPE_INT,.typeJsonName="int"} };
    }

    inline sierra::odyplan::v2::core::logical::port::Port createDoublePort() {
        return { sierra::odyplan::v2::core::logical::port::PortDescriptor {.typeId=TYPE_DOUBLE,.typeJsonName="double"} };
    }

    class ConstNumberSourceOperator final : public core::logical::opr::TypedSourceOperator {
    public:
        ConstNumberSourceOperator() : core::logical::opr::TypedSourceOperator(createIntPort()) {
        }
    };
} // namespace sierra::odyplan::v2::example::logical_plan

#endif //SIERRA_ODYPLAN_V2_EXAMPLE_LOGICALPLAN_CONSTNUMBERSOURCEOPERATOR_H