#ifndef SIERRA_ODYPLAN_V3_CORE_RUNTIME_TASK_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_CORE_RUNTIME_TASK_RUNTIME_HPP

#include <iostream>
#include <ostream>

#include <core/runtime/thread/compute_thread_pool.h>
#include <core/runtime/thread/io_thread_pool.h>

using ComputeTaskHandle = std::uint64_t;

inline sierra::odyplan::v3::core::runtime::thread::ComputeThreadPool& gComputeThreadPool() {
    static sierra::odyplan::v3::core::runtime::thread::ComputeThreadPool instance;
    return instance;
}

inline std::mutex gComputeMutex;
inline std::unordered_map<ComputeTaskHandle, std::future<void>> gComputeFutures;
inline std::atomic_uint64_t gNextComputeHandle{0};

inline sierra::odyplan::v3::core::runtime::thread::IoThreadPool gIoThreadPool;

using IOTaskHandle = std::uint64_t;

namespace sierra::odyplan::v3::core::runtime {
    extern "C" {
        ComputeTaskHandle rtComputeTaskSpawn(void (*fn)());

        void rtComputeTaskJoin(ComputeTaskHandle ioTaskHandle);

        IOTaskHandle rtIOTaskSpawn(void (*fn)());

        void rtIOTaskJoin(IOTaskHandle ioTaskHandle);

        void rtSleepMs(std::int64_t ms);
    }
}

#endif //SIERRA_ODYPLAN_V3_CORE_RUNTIME_TASK_RUNTIME_HPP