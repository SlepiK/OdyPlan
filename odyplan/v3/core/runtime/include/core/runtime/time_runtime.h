#ifndef SIERRA_ODYPLAN_V3_CORE_RUNTIME_TIME_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_CORE_RUNTIME_TIME_RUNTIME_HPP

#include <chrono>
#include <cstdint>
#include <core/shared/types/global_types.h>

namespace sierra::odyplan::v3::core::runtime {
    extern "C" {
    shared::types::Timestamp rtTimeNow();
    shared::types::Timestamp rtProgramTimeNow();
    }
}  // namespace sierra::odyplan::v3::core::runtime

#endif  // SIERRA_ODYPLAN_V3_CORE_RUNTIME_TIME_RUNTIME_HPP