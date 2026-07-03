#ifndef SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORT_H
#define SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORT_H

#include <memory>
#include <vector>

#include "port_descriptor.h"

namespace sierra::odyplan::v2::core::physical::port {
    using Port = std::vector<PortDescriptor>;
    using Ports = std::vector<Port>;
}  // namespace sierra::odyplan::v2::core::physical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORT_H