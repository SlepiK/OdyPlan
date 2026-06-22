#ifndef SIERRA_ODYPLAN_V1_CORE_EDGE_TYPEDEDGE_H
#define SIERRA_ODYPLAN_V1_CORE_EDGE_TYPEDEDGE_H

#include "core/operator/flow/consumer.h"
#include "core/operator/flow/emitter.h"
#include "core/tuple/tuple.h"
#include "edge.h"

#include <vector>

namespace sierra::odyplan::v1::core::edge {
    template <tuple::Tuple TEdgeTuple>
    class TypedEdge : public IEdge, public opr::flow::IEmitter<TEdgeTuple> {
    public:
        void addSink(opr::flow::IConsumer<TEdgeTuple>& sinkOperator) {
            mSinks.push_back(&sinkOperator);
        }

        void emit(const TEdgeTuple& value) override {
            for (auto* opr : mSinks) {
                opr->consume(value);
            }
        }

    private:
        std::vector<opr::flow::IConsumer<TEdgeTuple>*> mSinks;
    };
}  // namespace sierra::odyplan::v1::core::edge

#endif  // SIERRA_ODYPLAN_V1_CORE_EDGE_TYPEDEDGE_H