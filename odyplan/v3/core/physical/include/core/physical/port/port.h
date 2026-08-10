#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_PORT_PORT_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_PORT_PORT_HPP

#include <core/physical/port/port_description.h>
#include <vector>

namespace sierra::odyplan::v3::core::physical::port {
    using Port = std::vector<PortDescription>;
    using Ports = std::vector<Port>;
}  // namespace sierra::odyplan::v3::core::physical::port

#endif  // SIERRA_ODYPLAN_V3_CORE_PHYSICAL_PORT_PORT_HPP