#include <core/internal/threads/io_thread_pool.h>

namespace sierra::odyplan::v3::core::internal::threads {
    IoThreadPool::~IoThreadPool() {
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

    std::uint64_t IoThreadPool::submitTask(Task task) {
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
            workerLoop(workerId);
        });
        return workerId;
    }

    void IoThreadPool::joinWorker(const std::size_t workerId) {
        std::unique_lock<std::mutex> lock(mMutex);
        mDoneCV.wait(lock, [this, workerId] {
            return mCompletion.at(workerId);
        });
    }

    void IoThreadPool::workerLoop(std::size_t workerId) {
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
}