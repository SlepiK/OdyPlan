#ifndef SIERRA_ODYPLAN_V3_CORE_SHARED_TYPES_TYPE_ID_HPP
#define SIERRA_ODYPLAN_V3_CORE_SHARED_TYPES_TYPE_ID_HPP

#include <cstdint>

namespace sierra::odyplan::v3::core::shared::types {
    using TypeId = std::uint8_t;

    enum class BuiltinType : TypeId {
        BOOL = 0,
        INT8 = 1,
        INT16 = 2,
        INT32 = 3,
        INT64 = 4,
        FLOAT = 10,
        DOUBLE = 11,
        STRING = 20
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_SHARED_TYPES_TYPE_ID_HPP