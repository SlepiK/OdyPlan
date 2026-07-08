#ifndef SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_TYPE_H
#define SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_TYPE_H

#include <string_view>

#include "core/common/ident.h"

namespace sierra::odyplan::v2::core::common::type {
    using TypeId = std::size_t;

    enum class Type : TypeId {
        INT = 1,
        FLOAT = 2,
        DOUBLE = 3,
        STRING = 4,
        BOOL = 5
    };

    inline bool isPrintable(const TypeId typeId) {
        auto type = static_cast<Type>(typeId);
        return type == Type::INT || type == Type::FLOAT || type == Type::DOUBLE || type == Type::STRING;
    }
}  // namespace sierra::odyplan::v2::core::common::type

#endif  // SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_TYPE_H