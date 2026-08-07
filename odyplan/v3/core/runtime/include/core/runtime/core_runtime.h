#ifndef SIERRA_ODYPLAN_V3_CORE_RUNTIME_CORE_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_CORE_RUNTIME_CORE_RUNTIME_HPP

#include <atomic>

#include <core/runtime/task_runtime.h>

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

#endif //SIERRA_ODYPLAN_V3_CORE_RUNTIME_CORE_RUNTIME_HPP