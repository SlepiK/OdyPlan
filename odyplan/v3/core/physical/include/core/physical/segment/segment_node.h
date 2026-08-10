#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_SEGMENT_SEGMENT_NODE_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_SEGMENT_SEGMENT_NODE_HPP

#include <core/physical/flow/flow_consumer.h>
#include <core/physical/flow/flow_producer.h>

namespace sierra::odyplan::v3::core::physical::segment {
    struct SegmentNode {
        flow::IConsumer* asConsumer = nullptr;
        flow::IProducer* asProducer = nullptr;
        bool isBreaker = false;
    };
}  // namespace sierra::odyplan::v3::core::physical::segment

#endif  // SIERRA_ODYPLAN_V3_CORE_PHYSICAL_SEGMENT_SEGMENT_NODE_HPP