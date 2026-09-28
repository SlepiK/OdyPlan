#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_PROJECT_PHYSICAL_PROJECT_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_PROJECT_PHYSICAL_PROJECT_OPERATOR_HPP

#include <core/physical/operators/pipe/typed_pipe_operator.h>
#include <string>
#include <vector>

namespace sierra::odyplan::v3::basic::operators::pipe::project::physical {
    class ProjectOperator : public core::physical::operators::pipe::TypedPipeOperator {
    public:
        using Mapping = std::vector<std::size_t>;

        ProjectOperator(core::physical::port::Port inputPort,
                        core::physical::port::Port outputPort,
                        Mapping mapping);

        void consume(core::physical::segment::SegmentContext& segmentCtx,
                     core::shared::identifiers::PortId /*inputPort*/,
                     const core::physical::data::DataPackage& tuple,
                     const core::physical::segment::SegmentDataConsumers& consumers) override;

    private:
        Mapping mMapping;
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::project::physical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_PROJECT_PHYSICAL_PROJECT_OPERATOR_HPP