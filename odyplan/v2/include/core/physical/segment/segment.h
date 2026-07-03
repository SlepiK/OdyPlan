#ifndef SIERRA_ODYPLAN_V2_CORE_PHYSICAL_SEGMENT_SEGMENT_H
#define SIERRA_ODYPLAN_V2_CORE_PHYSICAL_SEGMENT_SEGMENT_H

#include "../operator/operator_ability.h"
#include "core/physical/operator/pipe/pipe_operator.h"
#include "core/physical/operator/sink/sink_operator.h"

namespace sierra::odyplan::v2::core::physical::segment {
    enum class SegmentType : uint8_t {
        NORMAL,
        BLOCKING /* End of a segment is a blocking operator */
    };

    struct Segment {
        SegmentType type = SegmentType::NORMAL;
        opr::IProducer* start = nullptr;
        std::vector<opr::IPipeOperator*> mid;
        opr::ISinkOperator* end = nullptr;
        opr::TypedBlockingPipeOperator* blockingTarget = nullptr;
    };
}  // namespace sierra::odyplan::v2::core::physical::segment

#endif  // SIERRA_ODYPLAN_V2_CORE_PHYSICAL_SEGMENT_SEGMENT_H