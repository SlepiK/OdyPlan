#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTDESCRIPTOR_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTDESCRIPTOR_H

#include "core/common/ident.h"

namespace sierra::odyplan::v2::core::logical::port {
    using PortTypeJsonName = std::string;

    struct PortDescriptor {
        common::identifier::PortTypeId typeId;
        PortTypeJsonName typeJsonName;
    };
}  // namespace sierra::odyplan::v2::core::logical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTDESCRIPTOR_H