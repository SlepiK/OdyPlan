#include <basic/operators/pipe/merge/physical/merge_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::merge::physical {
    MergeOperator::MergeOperator(core::physical::port::Port inputPort,
                                 core::physical::port::Port outputPort)
        : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)},
                            core::physical::port::Ports{std::move(outputPort)}) {}

    void MergeOperator::consume(core::physical::segment::SegmentContext& segmentCtx,
                                core::shared::identifiers::PortId /*inputPort*/,
                                const core::physical::data::DataPackage& tuple,
                                const core::physical::segment::SegmentDataConsumers& consumers) {
        consumers.front()(segmentCtx, tuple);
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::merge::physical