#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_PROJECT_PHYSICAL_PROJECT_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_PROJECT_PHYSICAL_PROJECT_OPERATOR_HPP

#include <string>
#include <vector>
#include <core/physical/operators/pipe/typed_pipe_operator.h>

namespace sierra::odyplan::v2::common::opr::pipe::physical {
    class ProjectOperator : public core::physical::operators::pipe::TypedPipeOperator {
    public:
        using Mapping = std::vector<std::size_t>;

        ProjectOperator(core::physical::port::Port inputPort, core::physical::port::Port outputPort, Mapping mapping);

        void consume(core::physical::segment::SegmentContext &segmentCtx,
                     core::shared::identifiers::PortId /*inputPort*/, const core::physical::port::PortTuple &tuple,
                     const core::physical::segment::SegmentDataConsumers &consumers) override;

        void end(core::physical::segment::SegmentContext &segmentCtx, core::shared::identifiers::PortId inputPort,
                 const core::physical::segment::SegmentDataConsumers &consumers) override;

    private:
        Mapping mMapping;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_PROJECT_PHYSICAL_PROJECT_OPERATOR_HPP