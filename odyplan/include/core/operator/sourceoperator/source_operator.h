#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_SOURCEOPERATOR_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_SOURCEOPERATOR_H

#include "core/operator/operator.h"

namespace sierra::odyplan::v1::core::opr {
    class ISourceOperator : public IOperator {
    public:
        ISourceOperator() = default;
        ~ISourceOperator() override = default;

        ISourceOperator(const ISourceOperator&) = delete;
        ISourceOperator& operator=(const ISourceOperator&) = delete;

        ISourceOperator(ISourceOperator&&) = delete;
        ISourceOperator& operator=(ISourceOperator&&) = delete;

        virtual bool produce() = 0;
    };

    template <typename T>
    concept SourceOperatorType =
        HasOperatorNeededFields<T> &&
        std::derived_from<T, ISourceOperator> &&
        std::same_as<typename T::Input, port::NullPort> &&
        port::PortType<typename T::Output>;
}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_SOURCEOPERATOR_H