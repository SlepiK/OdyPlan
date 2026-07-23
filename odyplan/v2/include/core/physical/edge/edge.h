#ifndef SIERRA_ODYPLAN_V2_CORE_PHYSICAL_EDGE_EDGE_H
#define SIERRA_ODYPLAN_V2_CORE_PHYSICAL_EDGE_EDGE_H

#include "core/common/ident.h"

namespace sierra::odyplan::v2::core::physical::edge {
    struct Edge {
        common::identifier::PortId fromOperatorOutputPortId;
        common::identifier::OperatorId toOperatorId;
        common::identifier::PortId toOperatorInputPortId;
    };

}  // namespace sierra::odyplan::v2::core::physical::chain

#endif  // SIERRA_ODYPLAN_V2_CORE_PHYSICAL_EDGE_EDGE_H