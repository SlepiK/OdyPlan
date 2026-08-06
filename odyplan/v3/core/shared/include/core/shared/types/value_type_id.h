#ifndef SIERRA_ODYPLAN_V3_CORE_SHARED_TYPES_VALUE_TYPE_ID_HPP
#define SIERRA_ODYPLAN_V3_CORE_SHARED_TYPES_VALUE_TYPE_ID_HPP

#include <cstdint>

namespace sierra::odyplan::v3::core::shared::types {
    using ValueTypeId = std::uint8_t;

    enum class BuiltinValueType : ValueTypeId {
        BOOL = 0,
        INT8 = 1,
        INT16 = 2,
        INT32 = 3,
        INT64 = 4,
        FLOAT = 10,
        DOUBLE = 11,
        STRING = 20
    };

    inline bool isBuiltinValueTypePrintable(const BuiltinValueType type) {
        return type == BuiltinValueType::BOOL || type == BuiltinValueType::INT8 || type == BuiltinValueType::INT16 ||
               type == BuiltinValueType::INT32 || type == BuiltinValueType::INT64 || type == BuiltinValueType::FLOAT ||
               type == BuiltinValueType::DOUBLE || type == BuiltinValueType::STRING;
    }
}

#endif //SIERRA_ODYPLAN_V3_CORE_SHARED_TYPES_VALUE_TYPE_ID_HPP