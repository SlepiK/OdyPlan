#ifndef SIERRA_ODYPLAN_V2_CORE_INTERNAL_THREAD_IOPOOL_H
#define SIERRA_ODYPLAN_V2_CORE_INTERNAL_THREAD_IOPOOL_H

#include <condition_variable>
#include <cstdint>
#include <mutex>

#include "task.h"

namespace sierra::odyplan::v2::core::internal::thread {
    class IOPool {
    public:
        IOPool() = default;

        ~IOPool() {
            {
                const std::scoped_lock lock(mMutex);
                mShutdown = true;
            }

            mWakeUpCV.notify_all();

            for (auto& worker : mWorkers) {
                if (worker.joinable()) {
                    worker.join();
                }
            }
        }

        IOPool(const IOPool&) = delete;
        IOPool& operator=(const IOPool&) = delete;
        IOPool(IOPool&&) = delete;
        IOPool& operator=(IOPool&&) = delete;


        std::uint64_t submitTask(Task task) {
            std::unique_lock<std::mutex> lock(mMutex);

            if (!mIdleWorkers.empty()) {
                const auto workerId = mIdleWorkers.back();
                mIdleWorkers.pop_back();
                mPendingTasks[workerId] = std::move(task);
                mCompletion[workerId] = false;
                lock.unlock();
                mWakeUpCV.notify_all();
                return workerId;
            }

            const auto workerId = mNextWorkerId++;
            mPendingTasks[workerId] = std::move(task);
            mCompletion[workerId] = false;
            lock.unlock();

            mWorkers.emplace_back([this, workerId] {
                workerMainLoop(workerId);
            });
            return workerId;
        }

        void joinTask(const std::uint64_t workerId) {
            std::unique_lock<std::mutex> lock(mMutex);
            mDoneCV.wait(lock, [this, workerId] {
                return mCompletion.at(workerId);
            });
        }

    private:
        std::mutex mMutex;
        std::condition_variable mWakeUpCV;
        std::condition_variable mDoneCV;
        std::vector<std::thread> mWorkers;
        std::unordered_map<std::uint64_t, Task> mPendingTasks;
        std::unordered_map<std::uint64_t, bool> mCompletion;
        std::vector<std::uint64_t> mIdleWorkers;
        std::uint64_t mNextWorkerId = 0;
        bool mShutdown = false;

        void workerMainLoop(const std::uint64_t workerId) {
            while (true) {
                Task task;
                {
                    std::unique_lock<std::mutex> lock(mMutex);
                    mWakeUpCV.wait(lock, [this, workerId] {
                        return mShutdown || mPendingTasks.at(workerId) != nullptr;
                    });

                    if (mShutdown && mPendingTasks.at(workerId) == nullptr) {
                        return;
                    }

                    task = std::move(mPendingTasks.at(workerId));
                    mPendingTasks[workerId] = nullptr;
                }

                task();

                {
                    const std::scoped_lock lock(mMutex);
                    mCompletion[workerId] = true;
                    mIdleWorkers.push_back(workerId);
                }

                mDoneCV.notify_all();
            }
        }
    };
}

#endif //SIERRA_ODYPLAN_V2_CORE_INTERNAL_THREAD_IOPOOL_H