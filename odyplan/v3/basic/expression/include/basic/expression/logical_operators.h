#ifndef SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_LOGICAL_OPERATORS_HPP
#define SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_LOGICAL_OPERATORS_HPP
#include <cstdint>

namespace sierra::odyplan::v3::basic::expression {
    enum class LogicalOp : std::uint8_t {
        AND,
        OR,
        NOT,
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_LOGICAL_OPERATORS_HPP