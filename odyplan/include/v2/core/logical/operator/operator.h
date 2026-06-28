#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_OPERATOR_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_OPERATOR_H

#include <span>

#include "v2/core/logical/port/port_type_info.h"

namespace sierra::odyplan::v2::core::logical::opr {
    class IOperator {
    public:
        IOperator() = default;
        virtual ~IOperator() = default;

        IOperator(const IOperator&) = delete;
        IOperator& operator=(const IOperator&) = delete;

        IOperator(IOperator&&) = delete;
        IOperator& operator=(IOperator&&) = delete;

        [[nodiscard]] virtual std::span<const port::PortTypeInfo> inputs() const = 0;
        [[nodiscard]] virtual std::span<const port::PortTypeInfo> outputs() const = 0;
    };

}  // namespace sierra::odyplan::v2::core::logical::opr

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_OPERATOR_OPERATOR_H