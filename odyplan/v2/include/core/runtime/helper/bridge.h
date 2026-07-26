#ifndef SIERRA_ODYPLAN_V2_CORE_RUNTIME_HELPER_BRIDGE_H
#define SIERRA_ODYPLAN_V2_CORE_RUNTIME_HELPER_BRIDGE_H

#include <core/runtime/thread/iopool.h>
#include <core/runtime/thread/workpool.h>
#include <unordered_map>
#include <memory>
#include <iostream>

inline sierra::odyplan::v2::core::runtime::thread::IOPool gIOPool;
inline sierra::odyplan::v2::core::runtime::thread::WorkPool& gWorkPool() {
    static sierra::odyplan::v2::core::runtime::thread::WorkPool instance;
    return instance;
}

inline std::mutex gComputeMutex;
inline std::unordered_map<std::uint64_t, std::future<void>> gComputeFutures;
inline std::atomic_uint64_t gNextComputeHandle{0};

extern "C" {
    inline std::uint64_t rtTaskSpawn(void (*fn)(), const sierra::odyplan::v2::core::runtime::thread::TaskKind kind) {
        try {
            if (kind == sierra::odyplan::v2::core::runtime::thread::TaskKind::IO) {
                return gIOPool.submitTask([fn] {fn();});
            }

            const auto handle = gNextComputeHandle.fetch_add(1, std::memory_order_relaxed) | (1ULL << 63);
            auto future = gWorkPool().submitWork([fn] {fn();});
            {
                const std::scoped_lock lock(gComputeMutex);
                gComputeFutures.emplace(handle, std::move(future));
            }
            return handle;
        } catch (const std::exception& e) {
            std::cerr << "rtTaskSpawn: " << e.what() << '\n';
            std::abort();
        }
    }

    inline void rtTaskJoin(const std::uint64_t handle) {
        if ((handle & static_cast<std::uint64_t>(1ULL << 63)) != 0) {
            std::future<void> future;
            {
                const std::scoped_lock lock(gComputeMutex);
                auto it = gComputeFutures.find(handle);
                if (it == gComputeFutures.end()) {
                    return;
                }
                future = std::move(it->second);
                gComputeFutures.erase(it);
            }
            future.wait();
            return;
        }
        gIOPool.joinTask(handle);
    }
}

#endif //SIERRA_ODYPLAN_V2_CORE_RUNTIME_HELPER_BRIDGE_H