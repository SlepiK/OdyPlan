#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_DATA_DATA_CONSUMER_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_DATA_DATA_CONSUMER_HPP

#include <core/physical/data/data_package.h>
#include <core/physical/segment/segment_context.h>

namespace sierra::odyplan::v3::core::physical::segment {

    struct SegmentConsumerHandle {
        std::function<void(SegmentContext&, const data::DataPackage&)> consume;
        std::function<void(SegmentContext&, const data::PunctuationPackage&)> consumePunctuation;
    };

    using SegmentDataConsumers = std::vector<SegmentConsumerHandle>;
}  // namespace sierra::odyplan::v3::core::physical::segment

#endif  // SIERRA_ODYPLAN_V3_CORE_PHYSICAL_DATA_DATA_CONSUMER_HPP