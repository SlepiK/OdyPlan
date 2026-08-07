#ifndef SIERRA_ODYPLAN_V3_CORE_RUNTIME_THREAD_IO_THREAD_POOL_HPP
#define SIERRA_ODYPLAN_V3_CORE_RUNTIME_THREAD_IO_THREAD_POOL_HPP

#include <condition_variable>
#include <cstdint>
#include <mutex>

#include <core/runtime/task/task.h>

namespace sierra::odyplan::v3::core::runtime::thread {
    class IoThreadPool {
    public:
		using Task = task::Task;

        IoThreadPool() = default;

        ~IoThreadPool();

        IoThreadPool(const IoThreadPool&) = delete;
        IoThreadPool& operator=(const IoThreadPool&) = delete;
        IoThreadPool(IoThreadPool&&) = delete;
        IoThreadPool& operator=(IoThreadPool&&) = delete;

        std::size_t submitTask(Task task);

        void joinWorker(std::size_t workerId);

    private:
        std::mutex mMutex;
        std::condition_variable mWakeUpCV;
        std::condition_variable mDoneCV;
        std::vector<std::thread> mWorkers;
        std::unordered_map<std::size_t, Task> mPendingTasks;
        std::unordered_map<std::size_t, bool> mCompletion;
        std::vector<std::size_t> mIdleWorkers;
        std::size_t mNextWorkerId = 0;
        bool mShutdown = false;

        void workerLoop(std::size_t workerId);
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_RUNTIME_THREAD_IO_THREAD_POOL_HPP