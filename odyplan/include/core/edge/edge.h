#ifndef SIERRA_ODYPLAN_V1_CORE_EDGE_EDGE_H
#define SIERRA_ODYPLAN_V1_CORE_EDGE_EDGE_H

#include <cstddef>
#include <memory>
#include <typeindex>
#include <core/operator/operator.h>
#include <core/operator/flow/emitter.h>

namespace sierra::odyplan::v1::core::edge {
    class Edge {
    public:
        Edge(const std::size_t source, const std::size_t sink, const std::size_t targetPortIndex) : mSource(source), mSink(sink), mTargetPortIndex(targetPortIndex) {}

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

        [[nodiscard]] std::size_t targetPortIndex() const {
            return mTargetPortIndex;
        }

        [[nodiscard]] virtual std::type_index payloadType() const = 0;

        [[nodiscard]] virtual std::unique_ptr<opr::flow::IEmitter> bind(opr::IOperator* sourceOperator, opr::IOperator* targetOperator) const = 0;

    private:
        std::size_t mSource;
        std::size_t mSink;
        std::size_t mTargetPortIndex;
    };
}  // namespace sierra::odyplan::v1::core::edge

#endif  // SIERRA_ODYPLAN_V1_CORE_EDGE_EDGE_H