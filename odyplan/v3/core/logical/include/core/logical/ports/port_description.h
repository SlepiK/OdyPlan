#ifndef ODYPLAN_V3_CORE_LOGICAL_PORT_PORT_DESCRIPTION_HPP
#define ODYPLAN_V3_CORE_LOGICAL_PORT_PORT_DESCRIPTION_HPP

namespace odyplan::v3::core::logical::port {
    struct PortDescription {

        bool operator==(const PortDescription&) const = default;
    };
}

#endif //ODYPLAN_V3_CORE_LOGICAL_PORT_PORT_DESCRIPTION_HPP