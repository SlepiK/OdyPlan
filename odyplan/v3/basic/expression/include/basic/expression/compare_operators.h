#ifndef SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_COMPARE_OPERATORS_HPP
#define SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_COMPARE_OPERATORS_HPP
#include <cstdint>

namespace sierra::odyplan::v3::basic::expression {
    enum class CompareOp : std::uint8_t {
        EQ,  // EQUAL
        NE,  // NOT_EQUAL
        LT,  // LESS_THAN
        LE,  // LESS_THAN_OR_EQUAL
        GT,  // GREATER_THAN
        GE,  // GREATER_THAN_OR_EQUAL
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_COMPARE_OPERATORS_HPP