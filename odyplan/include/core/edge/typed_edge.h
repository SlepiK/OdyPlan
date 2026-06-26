#ifndef SIERRA_ODYPLAN_V1_CORE_EDGE_TYPEDEDGE_H
#define SIERRA_ODYPLAN_V1_CORE_EDGE_TYPEDEDGE_H

#include "core/operator/ingest/ingest.h"
#include "core/operator/pipeoperator/pipe_operator.h"
#include "core/operator/sinkoperator/sink_operator.h"
#include "core/operator/sourceoperator/source_operator.h"
#include "core/port/port.h"
#include "edge.h"

#include <cstddef>
#include <typeindex>

namespace sierra::odyplan::v1::core::edge {
    template <typename From, typename To, std::size_t ToPortIndex>
        requires(std::derived_from<From, opr::IOperator> && std::derived_from<To, opr::IOperator> &&
                 (std::derived_from<From, opr::ISourceOperator> ||
                  std::derived_from<From, opr::IPipeOperator>) &&
                 (std::derived_from<To, opr::IPipeOperator> ||
                  std::derived_from<To, opr::ISinkOperator>) &&
                 (opr::HasOperatorNeededFields<From> && opr::HasOperatorNeededFields<To>))
    class TypedEdge final : public Edge {
    public:
        TypedEdge(std::size_t from, std::size_t to) : Edge(from, to, ToPortIndex) {}

        [[nodiscard]] std::type_index payloadType() const override {
            // return std::type_index(typeid(From::Output));
            return std::type_index(typeid(From));
        }

        [[nodiscard]] std::unique_ptr<opr::flow::IEmitter>
        bind(opr::IOperator* rawFrom, opr::IOperator* rawTo) const override {
            auto to = static_cast<To*>(rawTo);
            auto from = static_cast<From*>(rawFrom);

            using edgePortType = From::Output;

            opr::flow::IEmitting<edgePortType>* emittingSource = from;
            // NOLINTNEXTLINE(misc-const-correctness)
            opr::ingest::IIngestPort<ToPortIndex, edgePortType>* ingestPort = to;

            auto emitter =
                std::make_unique<opr::flow::BoundEmitter<ToPortIndex, edgePortType>>(ingestPort);
            emittingSource->setEmitter(emitter.get());
            return emitter;
        }
    };
}  // namespace sierra::odyplan::v1::core::edge

#endif  // SIERRA_ODYPLAN_V1_CORE_EDGE_TYPEDEDGE_H