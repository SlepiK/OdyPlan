#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTINFO_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTINFO_H

#include "port.h"
#include "port_type_info.h"

namespace sierra::odyplan::v2::core::logical::port {
    template <PortTypeInfo T>
    struct PortInfo;

    template <PortTypeInfo... Ts>
    struct PortInfo<Port<Ts...>> {
        static constexpr PortTypeInfo INFO{

        };
    };

}  // namespace sierra::odyplan::v2::core::logical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTINFO_H