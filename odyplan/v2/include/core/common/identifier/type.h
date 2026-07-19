#ifndef SIERRA_ODYPLAN_V2_CORE_COMMON_IDENTIFIER_TYPE_H
#define SIERRA_ODYPLAN_V2_CORE_COMMON_IDENTIFIER_TYPE_H

namespace sierra::odyplan::v2::core::common::identifier {
    using TypeId = std::uint8_t;

    enum class Type : TypeId {
        BOOL = 0,
        INT8 = 1,
        INT16 = 2,
        INT32 = 3,
        INT64 = 4,
        FLOAT = 10,
        DOUBLE = 11,
        STRING = 20
    };

    inline bool isTypePrintable(const TypeId typeId) {
        const auto type = static_cast<Type>(typeId);
        return type == Type::BOOL || type == Type::INT8 || type == Type::INT16 || type == Type::INT32 || type == Type::INT64 || type == Type::FLOAT || type == Type::DOUBLE || type == Type::STRING;
    }
}  // namespace sierra::odyplan::v2::core::common::identifier

#endif  // SIERRA_ODYPLAN_V2_CORE_COMMON_IDENTIFIER_TYPE_H