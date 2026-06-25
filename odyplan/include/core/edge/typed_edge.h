#ifndef SIERRA_ODYPLAN_V1_CORE_EDGE_TYPEDEDGE_H
#define SIERRA_ODYPLAN_V1_CORE_EDGE_TYPEDEDGE_H

#include "core/port/port.h"
#include "edge.h"

#include <cstddef>
#include <typeindex>
#include "core/operator/ingest/ingest.h"

namespace sierra::odyplan::v1::core::edge {
    template <typename From, typename To, std::size_t ToPortIndex>
        requires (std::derived_from<From, opr::IOperator> && std::derived_from<To, opr::IOperator> &&
            (std::derived_from<From, opr::ISourceOperator> || std::derived_from<From, opr::IPipeOperator>) &&
            (std::derived_from<To, opr::IPipeOperator> || std::derived_from<To, opr::ISinkOperator>) &&
            (opr::OperatorHandleHasPorts<From> && opr::OperatorHandleHasPorts<To>))
    class TypedEdge final : public Edge {
    public:
        TypedEdge(std::size_t from, std::size_t to) : Edge(from, to, ToPortIndex) {}

        [[nodiscard]] std::type_index payloadType() const override {
            return std::type_index(typeid(From::Output));
        }

        [[nodiscard]] std::unique_ptr<opr::flow::IEmitter> bind(From* iFrom, To* iTo) {
            using edgePortType = From::Output;

            opr::flow::IEmitting<edgePortType>* emittingSource = iFrom;
            opr::ingest::IIngestPort<ToPortIndex, edgePortType>* ingestPort = iTo;

            auto emitter = std::make_unique<opr::flow::BoundEmitter<ToPortIndex, edgePortType>>(ingestPort);
            emittingSource->setEmitter(emitter.get());
            return std::make_unique<opr::flow::Emitter<edgePortType>>(std::move(emitter));
        }
    };
}  // namespace sierra::odyplan::v1::core::edge

#endif  // SIERRA_ODYPLAN_V1_CORE_EDGE_TYPEDEDGE_H