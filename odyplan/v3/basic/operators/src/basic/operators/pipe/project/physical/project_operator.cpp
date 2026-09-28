#include <basic/operators/pipe/project/physical/project_operator.h>
#include <stdexcept>

namespace sierra::odyplan::v3::basic::operators::pipe::project::physical {
    ProjectOperator::ProjectOperator(core::physical::port::Port inputPort,
                                     core::physical::port::Port outputPort,
                                     Mapping mapping)
        : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)},
                            core::physical::port::Ports{std::move(outputPort)}),
          mMapping(std::move(mapping)) {}

    void ProjectOperator::consume(core::physical::segment::SegmentContext& segmentCtx,
                                  core::shared::identifiers::PortId /*inputPort*/,
                                  const core::physical::data::DataPackage& tuple,
                                  const core::physical::segment::SegmentDataConsumers& consumers) {
        core::physical::data::DataPackage projected(tuple.timeInterval);
        projected.reserve(mMapping.size());

        for (const auto index : mMapping) {
            projected.pushBack(tuple.at(index));
        }

        consumers.front().consume(segmentCtx, projected);
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::project::physical