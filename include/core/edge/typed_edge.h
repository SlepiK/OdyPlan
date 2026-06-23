#ifndef SIERRA_ODYPLAN_V1_CORE_EDGE_TYPEDEDGE_H
#define SIERRA_ODYPLAN_V1_CORE_EDGE_TYPEDEDGE_H

#include "core/tuple/tuple.h"
#include "edge.h"

#include <cstddef>
#include <typeindex>

namespace sierra::odyplan::v1::core::edge {
    template <tuple::Tuple T>
    class TypedEdge final : public Edge {
    public:
        TypedEdge(std::size_t source, std::size_t target) : Edge(source, target) {}

        [[nodiscard]] std::type_index payloadType() const override {
            return std::type_index(typeid(T));
        }
    };
}  // namespace sierra::odyplan::v1::core::edge

#endif  // SIERRA_ODYPLAN_V1_CORE_EDGE_TYPEDEDGE_H