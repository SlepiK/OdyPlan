#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_SEGMENT_SEGMENT_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_SEGMENT_SEGMENT_HPP

#include <core/physical/segment/segment_node.h>
#include <core/physical/port/port.h>

namespace sierra::odyplan::v3::core::physical::segment {
    struct Segment {
        std::vector<SegmentNode> nodes;
        port::Ports exitSchemas;
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_PHYSICAL_SEGMENT_SEGMENT_HPP