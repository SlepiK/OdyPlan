#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTREF_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTREF_H

#include <cstddef>

namespace sierra::odyplan::v2::core::logical::port {
    using OperatorId = std::size_t;
    using PortId = std::size_t;

    struct PortRef {
        OperatorId operatorId;
        PortId portId;
    };
}  // namespace sierra::odyplan::v2::core::logical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTREF_H