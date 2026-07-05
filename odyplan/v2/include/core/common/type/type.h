#ifndef SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_TYPE_H
#define SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_TYPE_H

#include "core/common/ident.h"

namespace sierra::odyplan::v2::core::common::type {
    enum TYPE : identifier::PortTypeId {
        INT = 1,
        FLOAT = 2,
        DOUBLE = 3,
        STRING = 4,
    };

    inline bool isPrintable(const identifier::PortTypeId typeId) {
        return typeId == INT || typeId == FLOAT || typeId == DOUBLE || typeId == STRING;
    }
}  // namespace sierra::odyplan::v2::core::common::type

#endif  // SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_TYPE_H