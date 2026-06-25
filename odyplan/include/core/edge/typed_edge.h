#ifndef SIERRA_ODYPLAN_V1_CORE_EDGE_TYPEDEDGE_H
#define SIERRA_ODYPLAN_V1_CORE_EDGE_TYPEDEDGE_H

#include "core/port/port.h"
#include "edge.h"

#include <cstddef>
#include <typeindex>
#include <core/operator/ingest/ingest.h>

namespace sierra::odyplan::v1::core::edge {
    inline constexpr std::size_t K_MAX_INGEST_SIDES = 8;

    template <typename FromOpt, typename ToOpt, std::size_t Port, port::PortType T>
    class TypedEdge final : public Edge {
    public:
        TypedEdge(std::size_t source, std::size_t target, std::size_t targetPortIndex) : Edge(source, target, targetPortIndex) {}

        [[nodiscard]] std::type_index payloadType() const override {
            return std::type_index(typeid(T));
        }

        [[nodiscard]] std::unique_ptr<opr::flow::IEmitter> bind(opr::IOperator* sourceOperator, opr::IOperator* targetOperator) {
            auto* cSourceOperator = static_cast<FromOpt*>(sourceOperator);
            auto* cTargetOperator = static_cast<ToOpt*>(targetOperator);

            opr::flow::IEmitting<T>* emittingSource = cSourceOperator;
            opr::ingest::IIngestPort<Port, T>* ingestPort = cTargetOperator;

            auto emitter = std::make_unique<opr::flow::BoundEmitter<T, Port>>(ingestPort);
            emittingSource->setEmitter(emitter.get());
            return std::make_unique<opr::flow::Emitter<T>>(std::move(emitter));
        }
    };
}  // namespace sierra::odyplan::v1::core::edge

#endif  // SIERRA_ODYPLAN_V1_CORE_EDGE_TYPEDEDGE_H