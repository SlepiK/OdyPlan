#ifndef SIERRA_ODYPLAN_V2_CORE_COMMON_IDENTIFIER_TYPE_H
#define SIERRA_ODYPLAN_V2_CORE_COMMON_IDENTIFIER_TYPE_H

#include <cstddef>

namespace sierra::odyplan::v2::core::common::identifier {
    using TypeId = std::size_t;

    enum class Type : TypeId {
        INT = 1,
        FLOAT = 2,
        DOUBLE = 3,
        STRING = 4,
        BOOL = 5
    };

    inline bool isTypePrintable(const TypeId typeId) {
        auto type = static_cast<Type>(typeId);
        return type == Type::INT || type == Type::FLOAT || type == Type::DOUBLE || type == Type::STRING;
    }
}  // namespace sierra::odyplan::v2::core::common::identifier

#endif  // SIERRA_ODYPLAN_V2_CORE_COMMON_IDENTIFIER_TYPE_H