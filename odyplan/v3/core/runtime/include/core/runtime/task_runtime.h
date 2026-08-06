#ifndef SIERRA_ODYPLAN_V3_CORE_RUNTIME_TASK_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_CORE_RUNTIME_TASK_RUNTIME_HPP

#include <iostream>
#include <ostream>

#include <core/internal/thread/compute_thread_pool.h>
#include <core/internal/thread/io_thread_pool.h>

using ComputeTaskHandle = std::uint64_t;

inline sierra::odyplan::v3::core::internal::thread::ComputeThreadPool& gComputeThreadPool() {
    static sierra::odyplan::v3::core::internal::thread::ComputeThreadPool instance;
    return instance;
}

inline std::mutex gComputeMutex;
inline std::unordered_map<ComputeTaskHandle, std::future<void>> gComputeFutures;
inline std::atomic_uint64_t gNextComputeHandle{0};

inline sierra::odyplan::v3::core::internal::thread::IoThreadPool gIoThreadPool;

using IOTaskHandle = std::uint64_t;

extern "C" {
    inline ComputeTaskHandle rtComputeTaskSpawn(void (*fn)()) {
        try {
            const auto handle = gNextComputeHandle.fetch_add(1, std::memory_order_relaxed) | (1ULL << 63ULL);
            auto future = gWorkPool().submitWork([fn] {fn();});
            {
                const std::scoped_lock lock(gComputeMutex);
                gComputeFutures.emplace(handle, std::move(future));
            }
            return handle;
        } catch (std::exception& e) {
            std::cerr << "rtComputeTaskSpawn:" << e.what() << '\n';
            std::abort();
        }
    }

    inline void rtComputeTaskJoin(ComputeTaskHandle ioTaskHandle) {
        std::future<void> future;
        {
            const std::scoped_lock lock(gComputeMutex);
            auto it = gComputeFutures.find(ioTaskHandle);
            if (it == gComputeFutures.end()) {
                return;
            }
            future = std::move(it->second);
            gComputeFutures.erase(it);
        }
        future.wait();
    }

    inline IOTaskHandle rtIOTaskSpawn(void (*fn)()) {
        return gIOPool.submitTask([fn] {fn();});
    }

    inline void rtIOTaskJoin(IOTaskHandle ioTaskHandle) {
        gIOPool.joinTask(ioTaskHandle);
    }

    inline void rtSleepMs(const std::int64_t ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
}

#endif //SIERRA_ODYPLAN_V3_CORE_RUNTIME_TASK_RUNTIME_HPP