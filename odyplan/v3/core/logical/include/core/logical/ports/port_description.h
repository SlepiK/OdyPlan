#ifndef SIERRA_ODYPLAN_V3_CORE_LOGICAL_PORTS_PORT_DESCRIPTION_HPP
#define SIERRA_ODYPLAN_V3_CORE_LOGICAL_PORTS_PORT_DESCRIPTION_HPP

#include <string>
#include <core/shared/types/value_type_id.h>

namespace sierra::odyplan::v3::core::logical::ports {
    struct PortDescription {
        using ValueTypeId = shared::types::ValueTypeId;
        using PortTypeJsonName = std::string;
        using PortFieldName = std::string;

        ValueTypeId valueTypeId;
        PortTypeJsonName typeJsonName;
        PortFieldName fieldName;

        bool operator==(const PortDescription&) const = default;
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_LOGICAL_PORTS_PORT_DESCRIPTION_HPP