#ifndef SIERRA_ODYPLAN_V2_CORE_COMMON_IDENTIFIER_OPERATORID_H
#define SIERRA_ODYPLAN_V2_CORE_COMMON_IDENTIFIER_OPERATORID_H

#include <cstddef>

namespace sierra::odyplan::v2::core::common::identifier {
    using OperatorTypeId = std::size_t;

    enum class Operator : OperatorTypeId {
        PROJECT = 10,
        SELECT = 11,
        PRINT = 20,
    };
}  // namespace sierra::odyplan::v2::core::common::identifier

#endif  // SIERRA_ODYPLAN_V2_CORE_COMMON_IDENTIFIER_OPERATORID_H