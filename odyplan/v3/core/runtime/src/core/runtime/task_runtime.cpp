#include <core/runtime/task_runtime.h>

namespace sierra::odyplan::v3::core::runtime {
    extern "C" {
    ComputeTaskHandle rtComputeTaskSpawn(void (*fn)()) {
        try {
            const auto handle =
                gNextComputeHandle.fetch_add(1, std::memory_order_relaxed) | (1ULL << 63ULL);
            auto future = gComputeThreadPool().submitTask([fn] { fn(); });
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

    void rtComputeTaskJoin(ComputeTaskHandle ioTaskHandle) {
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

    IOTaskHandle rtIOTaskSpawn(void (*fn)()) {
        return gIoThreadPool.submitTask([fn] { fn(); });
    }

    void rtIOTaskJoin(IOTaskHandle ioTaskHandle) {
        gIoThreadPool.joinWorker(ioTaskHandle);
    }

    void rtSleepMs(const std::int64_t ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
    }
}  // namespace sierra::odyplan::v3::core::runtime