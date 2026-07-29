#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_PROJECT_OPERATOR_PHYSICAL_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_PROJECT_OPERATOR_PHYSICAL_H

#include <string>
#include <vector>
#include "core/physical/operator/pipe/pipe_operator.h"

namespace sierra::odyplan::v2::common::opr::pipe::physical {
    class ProjectOperator : public core::physical::opr::TypedPipeOperator {
    public:
        using Mapping = std::vector<std::size_t>;

        ProjectOperator(core::physical::port::Port inputPort, core::physical::port::Port outputPort, Mapping mapping)
            : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)},
                                core::physical::port::Ports{std::move(outputPort)}),
              mMapping(std::move(mapping)) {
        }

        void consume(core::physical::segment::SegmentContext &segmentCtx,
                     core::common::identifier::PortId /*inputPort*/, const core::physical::port::PortTuple &tuple,
                     const core::physical::port::PortTupleConsumers &consumers) override {
            core::physical::port::PortTuple projected;
            projected.reserve(mMapping.size());
            projected.timestamp = tuple.timestamp;

            for (const auto index: mMapping) {
                if (index >= tuple.size()) {
                    throw std::invalid_argument("ProjectOperator: index out of range");
                }
                projected.push_back(tuple.at(index));
            }

            consumers.front()(segmentCtx, projected);
        }

        void end(core::physical::segment::SegmentContext &segmentCtx, const core::common::identifier::PortId inputPort,
                 const core::physical::port::PortTupleConsumers &consumers) override {
        }

    private:
        Mapping mMapping;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_PROJECT_OPERATOR_PHYSICAL_H