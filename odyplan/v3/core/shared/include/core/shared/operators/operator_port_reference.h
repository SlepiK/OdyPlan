#ifndef SIERRA_ODYPLAN_V3_CORE_SHARED_OPERATORS_OPERATOR_PORT_REFERENCE_HPP
#define SIERRA_ODYPLAN_V3_CORE_SHARED_OPERATORS_OPERATOR_PORT_REFERENCE_HPP

#include <core/shared/identifiers/operator_id.h>
#include <core/shared/identifiers/port_id.h>

namespace sierra::odyplan::v3::core::shared::operators {
    struct OperatorPortReference {
        using OperatorId = shared::identifiers::OperatorId;
        using PortId = shared::identifiers::PortId;
        OperatorId operatorId;
        PortId portId;
    };
}  // namespace sierra::odyplan::v3::core::shared::operators

#endif  // SIERRA_ODYPLAN_V3_CORE_SHARED_OPERATORS_OPERATOR_PORT_REFERENCE_HPP