#ifndef SIERRA_ODYPLAN_V2_CORE_COMMON_PORT_PORTREF_H
#define SIERRA_ODYPLAN_V2_CORE_COMMON_PORT_PORTREF_H

#include "core/common/ident.h"

namespace sierra::odyplan::v2::core::common::port {
    struct PortRef {
        identifier::OperatorId operatorId;
        identifier::PortId portId;
    };
}  // namespace sierra::odyplan::v2::core::common::port

#endif  // SIERRA_ODYPLAN_V2_CORE_COMMON_PORT_PORTREF_H