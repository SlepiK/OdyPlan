#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_PORT_PORT_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_PORT_PORT_HPP

#include <vector>

#include <core/physical/port/port_description.h>

namespace sierra::odyplan::v3::core::physical::port {
    using Port = std::vector<PortDescriptor>;
    using Ports = std::vector<Port>;
}

#endif //SIERRA_ODYPLAN_V3_CORE_PHYSICAL_PORT_PORT_HPP