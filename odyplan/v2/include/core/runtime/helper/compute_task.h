#ifndef SIERRA_ODYPLAN_V2_CORE_RUNTIME_HELPER_COMPUTETASK_H
#define SIERRA_ODYPLAN_V2_CORE_RUNTIME_HELPER_COMPUTETASK_H

#include <iostream>
#include <ostream>

#include "core/runtime/thread/workpool.h"

using ComputeTaskHandle = std::uint64_t;

inline sierra::odyplan::v2::core::runtime::thread::WorkPool& gWorkPool() {
    static sierra::odyplan::v2::core::runtime::thread::WorkPool instance;
    return instance;
}

inline std::mutex gComputeMutex;
inline std::unordered_map<ComputeTaskHandle, std::future<void>> gComputeFutures;
inline std::atomic_uint64_t gNextComputeHandle{0};

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
            std::cerr << "rtComputeTaskSpawn:" << e.what() << std::endl;
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
}

#endif //SIERRA_ODYPLAN_V2_CORE_RUNTIME_HELPER_COMPUTETASK_H