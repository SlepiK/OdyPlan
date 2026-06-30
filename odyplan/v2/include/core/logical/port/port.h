#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORT_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORT_H

#include <memory>
#include <vector>

#include "port_descriptor.h"

namespace sierra::odyplan::v2::core::logical::port {
    using Port = std::vector<PortDescriptor>;
    using Ports = std::vector<Port>;
}  // namespace sierra::odyplan::v2::core::logical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORT_H