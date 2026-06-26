#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_SINKOPERATOR_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_SINKOPERATOR_H

#include "core/operator/operator.h"
#include "core/port/port.h"

#include <concepts>

namespace sierra::odyplan::v1::core::opr {
    class ISinkOperator : public IOperator {
    public:
        ISinkOperator() = default;
        ~ISinkOperator() override = default;

        ISinkOperator(const ISinkOperator&) = delete;
        ISinkOperator& operator=(const ISinkOperator&) = delete;

        ISinkOperator(ISinkOperator&&) = delete;
        ISinkOperator& operator=(ISinkOperator&&) = delete;
    };

    template <typename T>
    concept SinkOperatorType =
        HasOperatorNeededFields<T> && std::derived_from<T, ISinkOperator> &&
        port::PortType<typename T::Input> && std::same_as<typename T::Output, port::NullPort>;
}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_SINKOPERATOR_H