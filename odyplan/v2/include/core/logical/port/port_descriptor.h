#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTDESCRIPTOR_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTDESCRIPTOR_H

namespace sierra::odyplan::v2::core::logical::port {
    using PortTypeId = std::size_t;
    using PortTypeJsonName = std::string;

    struct PortDescriptor {
        PortTypeId typeId;
        PortTypeJsonName typeJsonName;
    };
}  // namespace sierra::odyplan::v2::core::logical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTDESCRIPTOR_H