#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_PIPEOPERATOR_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_PIPEOPERATOR_H

#include "core/operator/operator.h"
#include "core/port/port.h"

#include <concepts>

namespace sierra::odyplan::v1::core::opr {
    class IPipeOperator : public IOperator {
    public:
        IPipeOperator() = default;
        ~IPipeOperator() override = default;

        IPipeOperator(const IPipeOperator&) = delete;
        IPipeOperator& operator=(const IPipeOperator&) = delete;

        IPipeOperator(IPipeOperator&&) = delete;
        IPipeOperator& operator=(IPipeOperator&&) = delete;
    };

    template <typename T>
    concept PipeOperatorType =
        HasOperatorNeededFields<T> && std::derived_from<T, IPipeOperator> &&
        port::PortsType<typename T::Input> && port::PortType<typename T::Output>;
}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_PIPEOPERATOR_H