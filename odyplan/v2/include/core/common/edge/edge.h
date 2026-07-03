#ifndef SIERRA_ODYPLAN_V2_CORE_COMMON_EDGE_EDGE_H
#define SIERRA_ODYPLAN_V2_CORE_COMMON_EDGE_EDGE_H

#include "core/common/port/port_ref.h"

namespace sierra::odyplan::v2::core::common::edge {
    struct Edge {
        port::PortRef fromOperator;
        port::PortRef toOperator;
    };
}  // namespace sierra::odyplan::v2::core::common::edge

#endif  // SIERRA_ODYPLAN_V2_CORE_COMMON_EDGE_EDGE_H