#ifndef SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_OPERATOR_TYPE_H
#define SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_OPERATOR_TYPE_H

#include <cstddef>

namespace sierra::odyplan::v2::core::common::type::opr {
    using TypeId = std::size_t;

    enum class Type : TypeId {
        INT = 1,
        FLOAT = 2,
        DOUBLE = 3,
        STRING = 4,
        BOOL = 5
    };
}  // namespace sierra::odyplan::v2::core::common::type

#endif  // SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_OPERATOR_TYPE_H