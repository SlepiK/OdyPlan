#ifndef SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_TYPE_H
#define SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_TYPE_H

#include <string_view>

#include "core/common/ident.h"

namespace sierra::odyplan::v2::core::common::type {
    using TypeId = std::size_t;

    enum class TYPE : TypeId {
        INT = 1,
        FLOAT = 2,
        DOUBLE = 3,
        STRING = 4,
        BOOL = 5
    };

    inline bool isPrintable(const TypeId typeId) {
        auto type = static_cast<TYPE>(typeId);
        return type == TYPE::INT || type == TYPE::FLOAT || type == TYPE::DOUBLE || type == TYPE::STRING;
    }
}  // namespace sierra::odyplan::v2::core::common::type

#endif  // SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_TYPE_H