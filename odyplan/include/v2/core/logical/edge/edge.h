#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_EDGE_EDGE_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_EDGE_EDGE_H

#include "v2/core/logical/port/port_ref.h"

namespace sierra::odyplan::v2::core::logical::edge {
    struct Edge {
        port::PortRef fromOperator;
        port::PortRef toOperator;
    };
}  // namespace sierra::odyplan::v2::core::logical::edge

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_EDGE_EDGE_H