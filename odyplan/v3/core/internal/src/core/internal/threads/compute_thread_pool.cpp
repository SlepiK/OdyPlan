#include <core/internal/threads/compute_thread_pool.h>

namespace sierra::odyplan::v3::core::internal::threads {
    ComputeThreadPool::ComputeThreadPool(const std::size_t poolSize)
        : mQueueMutexes(poolSize), mQueues(poolSize){
        mWorkers.reserve(poolSize);
        for (std::size_t i = 0; i < poolSize; ++i) {
            mWorkers.emplace_back([this, i] {});
        }
    }

    ComputeThreadPool::~ComputeThreadPool() {
        mShutdown.store(true, std::memory_order_release);
        mWakeCV.notify_all();
        for (auto& worker : mWorkers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    std::future<void> ComputeThreadPool::submitTask(Task task) {
        auto prom = std::make_shared<std::promise<void>>();
        std::future<void> fut = prom->get_future();

        Task wrapped = [task = std::move(task), prom] {
            task();
            prom->set_value();
        };

        const auto index = mRoundRobin.fetch_add(1, std::memory_order_relaxed) % mQueueMutexes.size();
        {
            const std::scoped_lock lock(mQueueMutexes.at(index));
            mQueues.at(index).push_front(std::move(wrapped));
        }

        mPending.fetch_add(1, std::memory_order_relaxed);
        mWakeCV.notify_all();
        return fut;
    }

    void ComputeThreadPool::workerLoop(std::size_t workerId) {
        std::mt19937 rng(std::random_device{}());

        while (!mShutdown.load(std::memory_order_acquire)) {
            Task task;
            if (tryPopOwn(workerId, task) || trySteal(workerId, task, rng)) {
                task();
                mPending.fetch_sub(1, std::memory_order_relaxed);
                continue;
            }

            std::unique_lock<std::mutex> lock(mWakeMutex);
            mWakeCV.wait_for(lock, std::chrono::milliseconds(1), [this] {
                return mShutdown.load(std::memory_order_acquire) || mPending.load(std::memory_order_relaxed);
            });
        }
    }

    bool ComputeThreadPool::tryPopOwn(const std::size_t workerId, Task& task) {
        const std::scoped_lock lock(mQueueMutexes.at(workerId));
        if (mQueues.at(workerId).empty()) {
            return false;
        }
        task = std::move(mQueues.at(workerId).front());
        mQueues.at(workerId).pop_front();
        return true;
    }

    bool ComputeThreadPool::trySteal(const std::size_t workerId, Task& task, std::mt19937& rng) {
        const auto scale = mQueues.size();
        if (scale <= 1) {
            return false;
        }

        std::uniform_int_distribution<std::size_t> dist(0, scale-1);
        for (std::size_t a = 0; a < scale; ++a) {
            const auto object = dist(rng);
            if (object == workerId) {
                continue;
            }

            const std::scoped_lock lock(mQueueMutexes.at(object));
            if (!mQueues.at(object).empty()) {
                task = std::move(mQueues.at(object).front());
                mQueues.at(object).pop_front();
                return true;
            }
        }

        return false;
    }
}