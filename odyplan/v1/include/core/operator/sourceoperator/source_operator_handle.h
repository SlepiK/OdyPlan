#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_SOURCEOPERATORHANDLE_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_SOURCEOPERATORHANDLE_H

#include "core/operator/operator_handle.h"
#include "core/port/port.h"
#include "source_operator.h"

namespace sierra::odyplan::v1::core::opr {
    template <typename Operator>
        requires(std::derived_from<Operator, ISourceOperator>)
    class SourceOperatorHandle final : public BaseOperatorHandle {
    public:
        using Input = port::NullPort;
        using Output = Operator::Output;
        using OperatorType = Operator;

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
        HasOperatorHandleNeededFields<T> && IsSourceHandle<std::remove_cvref_t<T>>::value &&
        std::same_as<typename T::Input, port::NullPort> && port::PortType<typename T::Output>;
}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_SOURCEOPERATORHANDLE_H