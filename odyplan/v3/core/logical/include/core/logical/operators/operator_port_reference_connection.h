#ifndef SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_OPERATOR_PORT_REFERENCE_CONNECTION_HPP
#define SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_OPERATOR_PORT_REFERENCE_CONNECTION_HPP

#include <core/logical/operators/operator_port_reference.h>

namespace sierra::odyplan::v3::core::logical::operators {
    struct OperatorPortReferenceConnection {
        OperatorPortReference fromOperatorPortReference;
        OperatorPortReference toOperatorPortReference;
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_LOGICAL_OPERATORS_OPERATOR_PORT_REFERENCE_CONNECTION_HPP