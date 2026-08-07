#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_OPERATOR_ROLE_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_OPERATOR_ROLE_HPP

#include <cstdint>

namespace sierra::odyplan::v3::core::physical::operators {
    enum class OperatorRole : std::uint8_t {
        SOURCE,
        PIPE,
        SINK
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_PHYSICAL_OPERATORS_OPERATOR_ROLE_HPP