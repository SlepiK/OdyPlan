#ifndef SIERRA_ODYPLAN_V3_CORE_RUNTIME_TASK_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_CORE_RUNTIME_TASK_RUNTIME_HPP

#include <core/runtime/thread/io_thread_pool.h>
#include <iostream>
#include <ostream>


inline std::mutex gComputeMutex;
inline std::atomic_uint64_t gNextComputeHandle{0};

inline sierra::odyplan::v3::core::runtime::thread::IoThreadPool gIoThreadPool;

using IOTaskHandle = std::uint64_t;

namespace sierra::odyplan::v3::core::runtime {
    extern "C" {
    IOTaskHandle rtIOTaskSpawn(void (*fn)());

    void rtIOTaskJoin(IOTaskHandle ioTaskHandle);

    void rtSleepMs(std::int64_t ms);
    }
}  // namespace sierra::odyplan::v3::core::runtime

#endif  // SIERRA_ODYPLAN_V3_CORE_RUNTIME_TASK_RUNTIME_HPP