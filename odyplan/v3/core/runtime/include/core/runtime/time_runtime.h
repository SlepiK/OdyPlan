#ifndef SIERRA_ODYPLAN_V3_CORE_RUNTIME_TIME_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_CORE_RUNTIME_TIME_RUNTIME_HPP

#include <chrono>
#include <cstdint>

#ifdef SIERRA_USER_PROGRAM_CLOCK
inline const auto gClock = std::chrono::steady_clock::now();
#endif

namespace sierra::odyplan::v3::core::runtime {
    extern "C" {
    std::int64_t rtNow();
    }
}  // namespace sierra::odyplan::v3::core::runtime

#endif  // SIERRA_ODYPLAN_V3_CORE_RUNTIME_TIME_RUNTIME_HPP