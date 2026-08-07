#ifndef SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_EXPRESSION_TYPE_HPP
#define SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_EXPRESSION_TYPE_HPP
#include <cstdint>

namespace sierra::odyplan::v3::basic::expression {
    enum class ExpressionType : std::uint8_t {
        FIELD_REFERENCE,
        CONSTANT,
        COMPARE,
        LOGICAL
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_EXPRESSION_TYPE_HPP