#ifndef SIERRA_ODYPLAN_V3_CORE_RUNTIME_TIME_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_CORE_RUNTIME_TIME_RUNTIME_HPP

#include <chrono>
#include <cstdint>

namespace sierra::odyplan::v3::core::runtime {
    extern "C" {
    std::int64_t rtTimeNow();
    std::int64_t rtProgramTimeNow();
    }
}  // namespace sierra::odyplan::v3::core::runtime

#endif  // SIERRA_ODYPLAN_V3_CORE_RUNTIME_TIME_RUNTIME_HPP