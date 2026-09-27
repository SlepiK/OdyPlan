#ifndef SIERRA_ODYPLAN_V3_CORE_SHARED_TYPES_GLOBAL_TYPES_HPP
#define SIERRA_ODYPLAN_V3_CORE_SHARED_TYPES_GLOBAL_TYPES_HPP

#include <cstdint>

namespace sierra::odyplan::v3::core::shared::types {
    using Timestamp = std::int64_t;
    constexpr Timestamp INFINITY_TIMESTAMP_SENTINEL = -std::numeric_limits<Timestamp>::max();
    using TiBreak = std::int64_t;


}

#endif //SIERRA_ODYPLAN_V3_CORE_SHARED_TYPES_GLOBAL_TYPES_HPP