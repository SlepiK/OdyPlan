#ifndef SIERRA_ODYPLAN_V2_CORE_RUNTIME_HELPER_BRIDGE_H
#define SIERRA_ODYPLAN_V2_CORE_RUNTIME_HELPER_BRIDGE_H

#include <core/runtime/thread/iopool.h>
#include <core/runtime/thread/workpool.h>
#include <unordered_map>
#include <memory>

inline sierra::odyplan::v2::core::runtime::thread::IOPool gIOPool;
inline sierra::odyplan::v2::core::runtime::thread::WorkPool gWorkPool;

inline std::mutex gComputeMutex;
inline std::unordered_map<std::uint64_t, std::future<void>> gComputeFutures;
inline std::atomic_int64_t gNextComputeHandle{0};

extern "C" {
    std::uint64_t rtTaskSpawn(void (*fn)(), const sierra::odyplan::v2::core::runtime::thread::TaskKind kind) {
        try {
            if (kind == sierra::odyplan::v2::core::runtime::thread::TaskKind::IO) {
                return gIOPool.submitTask([fn] {fn();});
            }

            const auto handle = gNextComputeHandle.fetch_add(1, std::memory_order_relaxed) | (1ull << 63);
            auto future = gWorkPool.submitWork([fn] {fn();});
            {
                std::lock_guard<std::mutex> lock(gComputeMutex);
                gComputeFutures.emplace(handle, std::move(future));
            }
            return handle;
        } catch (const std::exception& e) {
            std::fprintf(stderr, "rtTaskSpawn: %s\n", e.what());
            std::abort();
        }
    }

    void rtTaskJoin(const std::uint64_t handle) {
        if ((handle & (1ull << 63)) != 0) {
            std::future<void> future;
            {
                std::lock_guard<std::mutex> lock(gComputeMutex);
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