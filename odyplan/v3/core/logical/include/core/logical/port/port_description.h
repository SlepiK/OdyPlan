#ifndef SIERRA_ODYPLAN_V3_CORE_LOGICAL_PORT_PORT_DESCRIPTION_HPP
#define SIERRA_ODYPLAN_V3_CORE_LOGICAL_PORT_PORT_DESCRIPTION_HPP

#include <core/shared/types/value_type_id.h>
#include <string>

namespace sierra::odyplan::v3::core::logical::port {
    struct PortDescription {
        using ValueTypeId = shared::types::ValueTypeId;
        using PortFieldName = std::string;

        ValueTypeId valueTypeId;
        PortFieldName fieldName;

        bool operator==(const PortDescription&) const = default;
    };
}  // namespace sierra::odyplan::v3::core::logical::port

#endif  // SIERRA_ODYPLAN_V3_CORE_LOGICAL_PORT_PORT_DESCRIPTION_HPP