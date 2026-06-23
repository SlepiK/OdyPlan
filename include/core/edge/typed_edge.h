#ifndef SIERRA_ODYPLAN_V1_CORE_EDGE_TYPEDEDGE_H
#define SIERRA_ODYPLAN_V1_CORE_EDGE_TYPEDEDGE_H

#include "core/operator/flow/consumer.h"

namespace sierra::odyplan::v1::core::edge {
    template <class T>
    struct TypedEdge {
        std::size_t target;
    };
}  // namespace sierra::odyplan::v1::core::edge

#endif  // SIERRA_ODYPLAN_V1_CORE_EDGE_TYPEDEDGE_H