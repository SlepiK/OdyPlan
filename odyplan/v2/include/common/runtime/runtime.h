#ifndef SIERRA_ODYPLAN_V2_COMMON_RUNTIME_RUNTIME_H
#define SIERRA_ODYPLAN_V2_COMMON_RUNTIME_RUNTIME_H
#include <atomic>

inline std::atomic atStop{false};

extern "C" {
    inline void rtEnd() {
#ifdef SIERRA_RUNTIME_IO
        rtIOSinkFlushAll();
#endif
    }

    inline bool odyStop() {
        return atStop.load(std::memory_order_relaxed);
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_RUNTIME_RUNTIME_H