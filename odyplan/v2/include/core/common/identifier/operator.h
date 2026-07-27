#ifndef SIERRA_ODYPLAN_V2_CORE_COMMON_IDENTIFIER_OPERATORID_H
#define SIERRA_ODYPLAN_V2_CORE_COMMON_IDENTIFIER_OPERATORID_H

#include <cstdint>

namespace sierra::odyplan::v2::core::common::identifier {
    using OperatorTypeId = std::uint8_t;

    enum class Operator : OperatorTypeId {
        PROJECT = 10,
        SELECT = 11,
        MERGE = 12,
        PRINT = 20,
        READ = 21,
        WRITE = 22,
        WINDOW_COUNT = 30,
        WINDOW_TIME = 31,
        MEASURE = 40,
    };
}  // namespace sierra::odyplan::v2::core::common::identifier

#endif  // SIERRA_ODYPLAN_V2_CORE_COMMON_IDENTIFIER_OPERATORID_H