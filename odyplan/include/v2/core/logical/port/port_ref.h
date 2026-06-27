#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTREF_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTREF_H

#include "v2/core/identifier/identifier.h"

namespace sierra::odyplan::v2::core::logical::port {
    using OperatorId = identifier::Identifier<struct OperatorTag>;
    using PortId = identifier::Identifier<struct PortTag>;
    struct PortRef {
        OperatorId operatorId;
        PortId portId;
    };
}  // namespace sierra::odyplan::v2::core::logical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTREF_H