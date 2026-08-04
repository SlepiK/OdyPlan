#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_FLOW_FLOW_PRODUCER_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_FLOW_FLOW_PRODUCER_HPP

#include <core/physical/segment/segment_context.h>
#include <core/physical/data/data_consumer.h>

namespace sierra::odyplan::v3::core::physical::flow {
    class IProducer {
    public:
        IProducer() = default;
        virtual ~IProducer() = default;
        IProducer(IProducer const&) = delete;
        IProducer& operator =(IProducer const&) = delete;
        IProducer(IProducer&&) = delete;
        IProducer& operator=(IProducer&&) = delete;
        virtual void produce(segment::SegmentContext& segmentCtx, const data::DataConsumers& consumers) = 0;

        [[nodiscard]] virtual bool isComputeBound() const;
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_PHYSICAL_FLOW_FLOW_PRODUCER_HPP