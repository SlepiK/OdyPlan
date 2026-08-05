#ifndef SIERRA_ODYPLAN_V3_CORE_SHARED_OPERATORS_OPERATOR_PORT_REFERENCE_CONNECTION_HPP
#define SIERRA_ODYPLAN_V3_CORE_SHARED_OPERATORS_OPERATOR_PORT_REFERENCE_CONNECTION_HPP

#include <core/shared/operators/operator_port_reference.h>

namespace sierra::odyplan::v3::core::shared::operators {
    struct OperatorPortReferenceConnection {
        OperatorPortReference fromOperatorPortReference;
        OperatorPortReference toOperatorPortReference;
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_SHARED_OPERATORS_OPERATOR_PORT_REFERENCE_CONNECTION_HPP