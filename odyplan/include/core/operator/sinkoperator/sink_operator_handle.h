#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_SINKOPERATORHANDLE_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_SINKOPERATORHANDLE_H

#include "core/operator/operator_handle.h"
#include "core/port/port.h"
#include "sink_operator.h"

namespace sierra::odyplan::v1::core::opr {
    template <typename Operator>
        requires (std::derived_from<Operator, ISinkOperator>)
    class SinkOperatorHandle final : public BaseOperatorHandle {
    public:
        using Input = Operator::Input;
        using Output = port::NullPort;

        explicit SinkOperatorHandle(const std::size_t index) : BaseOperatorHandle(index) {}

        [[nodiscard]] std::type_index operatorType() const override {
            return std::type_index(typeid(Operator));
        }
    };

    template <typename T>
    struct IsSinkHandle : std::false_type {};

    template <typename Operator>
        requires std::derived_from<Operator, ISinkOperator>
    struct IsSinkHandle<SinkOperatorHandle<Operator>> : std::true_type {};

    template <typename T>
    concept SinkOperatorHandleType =
        OperatorHandleHasPorts<T> &&
        IsSinkHandle<std::remove_cvref_t<T>>::value &&
        port::PortType<typename T::Input> &&
        std::same_as<typename T::Output, port::NullPort>;
}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_SINKOPERATORHANDLE_H