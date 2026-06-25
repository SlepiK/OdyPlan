#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_SOURCEOPERATORHANDLE_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_SOURCEOPERATORHANDLE_H

#include "source_operator.h"
#include "core/port/port.h"
#include "core/operator/operator_handle.h"

namespace sierra::odyplan::v1::core::opr {
    template <typename Operator>
        requires (std::derived_from<Operator, ISourceOperator>)
    class SourceOperatorHandle final : public BaseOperatorHandle {
    public:
        using Input = port::NullPort;
        using Output = Operator::Output;

        explicit SourceOperatorHandle(const std::size_t index) : BaseOperatorHandle(index) {}

        [[nodiscard]] std::type_index operatorType() const override {
            return std::type_index(typeid(Operator));
        }
    };

    template <typename T>
    struct IsSourceHandle : std::false_type {};

    template <typename Operator>
        requires std::derived_from<Operator, ISourceOperator>
    struct IsSourceHandle<SourceOperatorHandle<Operator>> : std::true_type {};

    template <typename T>
    concept SourceOperatorHandleType =
        OperatorHandleHasPorts<T> &&
        IsSourceHandle<std::remove_cvref_t<T>>::value &&
        std::same_as<typename T::Input, port::NullPort> &&
        port::PortType<typename T::Output>;
}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_SOURCEOPERATORHANDLE_H