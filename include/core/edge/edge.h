#ifndef SIERRA_ODYPLAN_V1_CORE_EDGE_EDGE_H
#define SIERRA_ODYPLAN_V1_CORE_EDGE_EDGE_H

#include <cstddef>
#include <typeindex>

namespace sierra::odyplan::v1::core::edge {
    class Edge {
    public:
        Edge(const std::size_t source, const std::size_t sink) : mSource(source), mSink(sink) {}

        virtual ~Edge() = default;
        Edge(const Edge&) = delete;
        Edge& operator=(const Edge&) = delete;
        Edge(Edge&&) = delete;
        Edge& operator=(Edge&&) = delete;

        [[nodiscard]] std::size_t source() const {
            return mSource;
        }

        [[nodiscard]] std::size_t sink() const {
            return mSink;
        }

        [[nodiscard]] virtual std::type_index payloadType() const = 0;

    private:
        std::size_t mSource;
        std::size_t mSink;
    };
}  // namespace sierra::odyplan::v1::core::edge

#endif  // SIERRA_ODYPLAN_V1_CORE_EDGE_EDGE_H