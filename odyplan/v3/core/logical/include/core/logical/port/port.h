#ifndef SIERRA_ODYPLAN_V3_CORE_LOGICAL_PORT_PORT_HPP
#define SIERRA_ODYPLAN_V3_CORE_LOGICAL_PORT_PORT_HPP

#include <vector>
#include <core/logical/port/port_description.h>

namespace sierra::odyplan::v3::core::logical::port {
    using Port = std::vector<PortDescription>;
    using Ports = std::vector<Port>;
}

#endif //SIERRA_ODYPLAN_V3_CORE_LOGICAL_PORT_PORT_HPP