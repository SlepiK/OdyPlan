#ifndef SIERRA_ODYPLAN_V1_CORE_EDGE_EDGE_H
#define SIERRA_ODYPLAN_V1_CORE_EDGE_EDGE_H

#include <core/operator/flow/emitter.h>
#include <core/operator/operator.h>
#include <cstddef>
#include <memory>
#include <typeindex>

namespace sierra::odyplan::v1::core::edge {
    class Edge {
    public:
        Edge(const std::size_t from, const std::size_t to, const std::size_t toPortIndex)
            : mFrom(from), mTo(to), mToPortIndex(toPortIndex) {}

        virtual ~Edge() = default;
        Edge(const Edge&) = delete;
        Edge& operator=(const Edge&) = delete;
        Edge(Edge&&) = delete;
        Edge& operator=(Edge&&) = delete;

        [[nodiscard]] std::size_t from() const {
            return mFrom;
        }

        [[nodiscard]] std::size_t to() const {
            return mTo;
        }

        [[nodiscard]] std::size_t toPortIndex() const {
            return mToPortIndex;
        }

        [[nodiscard]] virtual std::type_index payloadType() const = 0;

        [[nodiscard]] virtual std::unique_ptr<opr::flow::IEmitter>
        bind(opr::IOperator* rawFrom, opr::IOperator* rawTo) const = 0;

    private:
        std::size_t mFrom;
        std::size_t mTo;
        std::size_t mToPortIndex;
    };
}  // namespace sierra::odyplan::v1::core::edge

#endif  // SIERRA_ODYPLAN_V1_CORE_EDGE_EDGE_H