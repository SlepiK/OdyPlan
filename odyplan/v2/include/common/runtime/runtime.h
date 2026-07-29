#ifndef SIERRA_ODYPLAN_V2_COMMON_RUNTIME_RUNTIME_H
#define SIERRA_ODYPLAN_V2_COMMON_RUNTIME_RUNTIME_H
#include <atomic>
#include "task/io/io_task.h"
#include "task/compute/compute_task.h"

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

    inline std::int64_t rtNow() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    inline void rtSleepMs(const std::int64_t ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_RUNTIME_RUNTIME_H