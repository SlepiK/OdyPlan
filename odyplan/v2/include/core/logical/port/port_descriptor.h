#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTDESCRIPTOR_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTDESCRIPTOR_H

#include <string>

#include "core/common/ident.h"
#include "../../common/identifier/type.h"

namespace sierra::odyplan::v2::core::logical::port {
    using PortTypeJsonName = std::string;
    using PortFieldName = std::string;

    struct PortDescriptor {
        common::identifier::TypeId typeId;
        PortTypeJsonName typeJsonName;
        PortFieldName fieldName;
    };
}  // namespace sierra::odyplan::v2::core::logical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTDESCRIPTOR_H