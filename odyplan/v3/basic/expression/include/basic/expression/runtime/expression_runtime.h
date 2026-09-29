#ifndef SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_RUNTIME_EXPRESSION_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_RUNTIME_EXPRESSION_RUNTIME_HPP
#include <core/shared/types/value_type_id.h>

namespace sierra::odyplan::v3::basic::expression::runtime {
    extern "C" {
        bool rtTypeEqual(core::shared::types::ValueTypeId typeId, const void* left, const void* right);
    }
}

#endif //SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_RUNTIME_EXPRESSION_RUNTIME_HPP