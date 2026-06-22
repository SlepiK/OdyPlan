#ifndef SIERRA_ODYPLAN_V1_CORE_EDGE_EDGE_H
#define SIERRA_ODYPLAN_V1_CORE_EDGE_EDGE_H

namespace sierra::odyplan::v1::core::edge {
    class IEdge {
    public:
        IEdge() = default;
        virtual ~IEdge() = default;

        IEdge(const IEdge&) = default;
        IEdge& operator=(const IEdge&) = default;

        IEdge(IEdge&&) = default;
        IEdge& operator=(IEdge&&) = default;
    };
}  // namespace sierra::odyplan::v1::core::edge

#endif  // SIERRA_ODYPLAN_V1_CORE_EDGE_EDGE_H