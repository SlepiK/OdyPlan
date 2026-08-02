#ifndef SIERRA_ODYPLAN_V3_CORE_SHARED_TYPES_OPERATOR_TYPE_ID_HPP
#define SIERRA_ODYPLAN_V3_CORE_SHARED_TYPES_OPERATOR_TYPE_ID_HPP

#include <cstdint>

namespace sierra::odyplan::v3::core::shared::operators {
    using OperatorTypeId = std::uint8_t;

    enum class BuiltinOperatorType : OperatorTypeId {
        PROJECT = 10,
        SELECT = 11,
        MERGE = 12,
        INTERVAL_JOIN = 13,
        PRINT = 20,
        READ = 21,
        WRITE = 22,
        WINDOW_COUNT = 30,
        WINDOW_TIME = 31,
        MEASURE = 40,
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_SHARED_TYPES_OPERATOR_TYPE_ID_HPP