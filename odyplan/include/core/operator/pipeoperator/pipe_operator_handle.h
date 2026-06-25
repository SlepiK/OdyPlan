#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_PIPEOPERATORHANDLE_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_PIPEOPERATORHANDLE_H

#include "core/operator/operator_handle.h"
#include "core/port/port.h"
#include "pipe_operator.h"

namespace sierra::odyplan::v1::core::opr {
    template <typename Operator>
        requires (std::derived_from<Operator, IPipeOperator>)
    class PipeOperatorHandle final : public BaseOperatorHandle {
    public:
        using Input = Operator::Input;
        using Output = Operator::Output;

        explicit PipeOperatorHandle(const std::size_t index) : BaseOperatorHandle(index) {}

        [[nodiscard]] std::type_index operatorType() const override {
            return std::type_index(typeid(Operator));
        }
    };

    template <typename T>
   struct IsPipeHandle : std::false_type {};

    template <typename Operator>
        requires std::derived_from<Operator, IPipeOperator>
    struct IsPipeHandle<PipeOperatorHandle<Operator>> : std::true_type {};

    template <typename T>
    concept PipeOperatorHandleType =
        OperatorHandleHasPorts<T> &&
        IsPipeHandle<std::remove_cvref_t<T>>::value &&
        port::PortsType<typename T::Input> &&
        port::PortType<typename T::Output>;
}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_PIPEOPERATORHANDLE_H