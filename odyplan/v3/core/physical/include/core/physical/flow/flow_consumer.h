#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_FLOW_FLOW_CONSUMER_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_FLOW_FLOW_CONSUMER_HPP

#include <core/physical/segment/segment_context.h>
#include <core/physical/data/data_consumer.h>
#include <core/physical/data/data_package.h>

namespace sierra::odyplan::v3::core::physical::flow {
    class IConsumer {
    public:
        IConsumer() = default;
        virtual ~IConsumer() = default;
        IConsumer(IConsumer const&) = delete;
        IConsumer& operator =(IConsumer const&) = delete;
        IConsumer(IConsumer&&) = delete;
        IConsumer& operator=(IConsumer&&) = delete;
        virtual void consume(segment::SegmentContext& segmentCtx, common::identifier::PortId inputPort, const data::DataPackage& data, const data::DataConsumers& consumers) = 0;
        virtual void end(segment::SegmentContext& segmentCtx, common::identifier::PortId inputPort, const data::DataConsumers& consumers) {
        }
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_PHYSICAL_FLOW_FLOW_CONSUMER_HPP