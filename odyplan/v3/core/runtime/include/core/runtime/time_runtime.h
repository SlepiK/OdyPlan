#ifndef SIERRA_ODYPLAN_V3_CORE_RUNTIME_TIME_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_CORE_RUNTIME_TIME_RUNTIME_HPP

#include <chrono>
#include <cstdint>

#ifdef SIERRA_USER_PROGRAM_CLOCK
inline const auto gClock = std::chrono::steady_clock::now();
#endif

extern "C" {
    inline std::int64_t rtNow() {
#ifdef SIERRA_USER_PROGRAM_CLOCK
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - gClock).count();
#else
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
#endif
    }
}

#endif //SIERRA_ODYPLAN_V3_CORE_RUNTIME_TIME_RUNTIME_HPP