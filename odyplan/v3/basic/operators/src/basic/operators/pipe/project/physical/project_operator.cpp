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
        core::physical::data::DataPackage projected;
        projected.reserve(mMapping.size());
        projected.timestamp = tuple.timestamp;

        for (const auto index : mMapping) {
            if (index >= tuple.size()) {
                throw std::invalid_argument("ProjectOperator: index out of range");
            }
            projected.pushBack(tuple.at(index));
        }

        consumers.front().consume(segmentCtx, projected);
    }

    void ProjectOperator::end(core::physical::segment::SegmentContext& segmentCtx,
                              const core::shared::identifiers::PortId inputPort,
                              const core::physical::segment::SegmentDataConsumers& consumers) {}
}  // namespace sierra::odyplan::v3::basic::operators::pipe::project::physical