#ifndef SIERRA_ODYPLAN_V2_CORE_PHYSICAL_SEGMENT_SEGMENT_H
#define SIERRA_ODYPLAN_V2_CORE_PHYSICAL_SEGMENT_SEGMENT_H

#include "core/physical/operator/internal/operator_ability.h"

namespace sierra::odyplan::v2::core::physical::segment {
    struct Segment {
        opr::internal::IProducer* start;
        std::vector<opr::internal::IConsumer*> midEnd;
    };
}  // namespace sierra::odyplan::v2::core::physical::segment

#endif  // SIERRA_ODYPLAN_V2_CORE_PHYSICAL_SEGMENT_SEGMENT_H