#ifndef SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_BINDABLE_BINDABLE_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_BINDABLE_BINDABLE_OPERATOR_HPP

#include <core/shared/identifiers/port_id.h>
#include <core/logical/port/port.h>

namespace sierra::odyplan::v3::core::logical::plan {
    class Plan;
}

namespace sierra::odyplan::v3::core::logical::operators::bindable {
    class IBindableOperator {
        friend class plan::Plan;
    public:
    	using PortId = shared::identifiers::PortId;
        using Port = port::Port;

        IBindableOperator() = default;

        virtual ~IBindableOperator() = default;

        IBindableOperator(const IBindableOperator&) = delete;

        IBindableOperator& operator=(const IBindableOperator&) = delete;

        IBindableOperator(IBindableOperator&&) = delete;

        IBindableOperator& operator=(IBindableOperator&&) = delete;
    protected:
        virtual void bindInput(PortId portId, Port input) = 0;

        virtual void validateInput() = 0;
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_BINDABLE_BINDABLE_OPERATOR_HPP