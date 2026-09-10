#ifndef SIERRA_ODYPLAN_V3_EXAMPLE_LOGICALPLAN_CONSTNUMBERSOURCEOPERATOR_HPP
#define SIERRA_ODYPLAN_V3_EXAMPLE_LOGICALPLAN_CONSTNUMBERSOURCEOPERATOR_HPP

#include <core/logical/operators/source/typed_source_operator.h>

namespace sierra::odyplan::v3::example::logical_plan {
    constexpr core::shared::types::ValueTypeId TYPE_INT = 1;
    constexpr core::shared::types::ValueTypeId TYPE_DOUBLE = 2;

    inline core::logical::port::Port createIntPort() {
        return {
            core::logical::port::PortDescription{.valueTypeId = TYPE_INT}};
    }

    inline core::logical::port::Port createDoublePort() {
        return {core::logical::port::PortDescription{.valueTypeId = TYPE_DOUBLE}};
    }

    class ConstNumberSourceOperator final
        : public core::logical::operators::source::TypedSourceOperator {
    public:
        static constexpr core::shared::types::OperatorTypeId TYPE_ID = 0;

        ConstNumberSourceOperator() : TypedSourceOperator(createIntPort()) {}

        [[nodiscard]] core::shared::types::OperatorTypeId getOperatorTypeId() const override {
            return TYPE_ID;
        }
    };

    class ConstDoubleNumberSourceOperator final
        : public core::logical::operators::source::TypedSourceOperator {
    public:
        static constexpr core::shared::types::OperatorTypeId TYPE_ID = 3;

        ConstDoubleNumberSourceOperator() : TypedSourceOperator(createDoublePort()) {}

        [[nodiscard]] core::shared::types::OperatorTypeId getOperatorTypeId() const override {
            return TYPE_ID;
        }
    };
}  // namespace sierra::odyplan::v3::example::logical_plan

#endif  // SIERRA_ODYPLAN_V3_EXAMPLE_LOGICALPLAN_CONSTNUMBERSOURCEOPERATOR_HPP