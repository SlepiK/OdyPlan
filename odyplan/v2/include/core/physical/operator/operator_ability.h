#ifndef SIERRA_ODYPLAN_V2_CORE_PHYSICAL_OPERATOR_OPERATORABILITY_H
#define SIERRA_ODYPLAN_V2_CORE_PHYSICAL_OPERATOR_OPERATORABILITY_H

#include <llvm/IR/IRBuilder.h>

#include "core/physical/port/port_tuple.h"
#include "core/physical/segment/segment_context.h"

namespace sierra::odyplan::v2::core::physical::opr {
    class IProducer {
    public:
        IProducer() = default;
        virtual ~IProducer() = default;
        IProducer(IProducer const&) = delete;
        IProducer& operator =(IProducer const&) = delete;
        IProducer(IProducer&&) = delete;
        IProducer& operator=(IProducer&&) = delete;
        virtual void produce(segment::SegmentContext& segmentCtx, port::PortTupleConsumer consumer) = 0;
    };

    class IConsumer {
    public:
        IConsumer() = default;
        virtual ~IConsumer() = default;
        IConsumer(IConsumer const&) = delete;
        IConsumer& operator =(IConsumer const&) = delete;
        IConsumer(IConsumer&&) = delete;
        IConsumer& operator=(IConsumer&&) = delete;
        virtual void consume(segment::SegmentContext& segmentCtx, const port::PortTuple& tuple, port::PortTupleConsumer consumer) = 0;
        virtual void end(segment::SegmentContext& segmentCtx, port::PortTupleConsumer consumer) = 0;
    };
}  // namespace sierra::odyplan::v2::core::physical::opr

#endif  // SIERRA_ODYPLAN_V2_CORE_PHYSICAL_OPERATOR_OPERATORABILITY_H