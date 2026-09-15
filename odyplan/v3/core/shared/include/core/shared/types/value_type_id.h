#ifndef SIERRA_ODYPLAN_V3_CORE_SHARED_TYPES_VALUE_TYPE_ID_HPP
#define SIERRA_ODYPLAN_V3_CORE_SHARED_TYPES_VALUE_TYPE_ID_HPP

#include <cstdint>
#include <stdexcept>

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
        STRING = 20,
        TIMESTAMP = 66,
        UNKNOWN = 255
    };

    constexpr bool isNumeric(const ValueTypeId type) {
        switch (static_cast<BuiltinValueType>(type)) {
            case BuiltinValueType::BOOL:
            case BuiltinValueType::INT8:
            case BuiltinValueType::INT16:
            case BuiltinValueType::INT32:
            case BuiltinValueType::INT64:
            case BuiltinValueType::FLOAT:
            case BuiltinValueType::DOUBLE:
            case BuiltinValueType::TIMESTAMP:
                return true;
            default:
                return false;
        }
        throw std::invalid_argument("isNumeric: unknown ValueTypeId/BuiltinValueType");
    }

    constexpr bool isTimestamp(const ValueTypeId type) {
        return static_cast<BuiltinValueType>(type) == BuiltinValueType::TIMESTAMP;
    }

    constexpr bool isIntegral(const ValueTypeId type) {
        switch (static_cast<BuiltinValueType>(type)) {
            case BuiltinValueType::BOOL:
            case BuiltinValueType::INT8:
            case BuiltinValueType::INT16:
            case BuiltinValueType::INT32:
            case BuiltinValueType::INT64:
            case BuiltinValueType::TIMESTAMP:
                return true;
            default:
                return false;
        }
        throw std::invalid_argument("isIntegral: unknown ValueTypeId/BuiltinValueType");
    }

    constexpr bool isBuiltinValueTypePrintable(const ValueTypeId type) {
        switch (static_cast<BuiltinValueType>(type)) {
            case BuiltinValueType::BOOL:
            case BuiltinValueType::INT8:
            case BuiltinValueType::INT16:
            case BuiltinValueType::INT32:
            case BuiltinValueType::INT64:
            case BuiltinValueType::FLOAT:
            case BuiltinValueType::DOUBLE:
            case BuiltinValueType::STRING:
            case BuiltinValueType::TIMESTAMP:
                return true;
            default:
                return false;
        }
        throw std::invalid_argument("isIntegral: unknown ValueTypeId/BuiltinValueType");
    }
}  // namespace sierra::odyplan::v3::core::shared::types

#endif  // SIERRA_ODYPLAN_V3_CORE_SHARED_TYPES_VALUE_TYPE_ID_HPP