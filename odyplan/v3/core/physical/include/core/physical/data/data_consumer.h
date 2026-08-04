#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_DATA_DATA_CONSUMER_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_DATA_DATA_CONSUMER_HPP

#include <core/physical/segment/segment_context.h>
#include <core/physical/data/data_package.h>

namespace sierra::odyplan::v3::core::physical::data {
    using DataConsumer = std::function<void(segment::SegmentContext& , const DataPackage&)>;
    using DataConsumers = std::vector<DataConsumer>;
}

#endif //SIERRA_ODYPLAN_V3_CORE_PHYSICAL_DATA_DATA_CONSUMER_HPP