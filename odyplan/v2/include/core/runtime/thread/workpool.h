#ifndef SIERRA_ODYPLAN_V2_CORE_RUNTIME_THREAD_WORKPOOL_H
#define SIERRA_ODYPLAN_V2_CORE_RUNTIME_THREAD_WORKPOOL_H
#include <condition_variable>
#include <deque>
#include <future>
#include <mutex>
#include <vector>

#include <random>
#include "task.h"

namespace sierra::odyplan::v2::core::runtime::thread {
    class WorkPool {
    public:
        explicit WorkPool(const std::size_t poolSize = std::max<std::size_t>(1, std::thread::hardware_concurrency()))
            : mQueues(poolSize),
                mQueueMutexes(poolSize) {
            mWorkers.reserve(poolSize);
            for (std::size_t i = 0; i < poolSize; ++i) {
                mWorkers.emplace_back([this, i] {});
            }
        }

        ~WorkPool() {
            mShutdown.store(true, std::memory_order_release);
            mWakeCV.notify_all();
            for (auto& worker : mWorkers) {
                if (worker.joinable()) {
                    worker.join();
                }
            }
        }

        std::future<void> submitWork(Task task) {
            auto prom = std::make_shared<std::promise<void>>();
            std::future<void> fut = prom->get_future();

            Task wrapped = [task = std::move(task), prom] {
                task();
                prom->set_value();
            };

            const auto index = mRoundRobin.fetch_add(1, std::memory_order_relaxed) % mQueueMutexes.size();
            {
                std::lock_guard<std::mutex> lock(mQueueMutexes[index]);
                mQueues[index].push_front(std::move(wrapped));
            }

            mPending.fetch_add(1, std::memory_order_relaxed);
            mWakeCV.notify_all();
            return fut;
        }

    private:
        std::mutex mWakeMutex;
        std::condition_variable mWakeCV;
        std::atomic_bool mShutdown{false};
        std::atomic<std::size_t> mRoundRobin{0};
        std::atomic<std::size_t> mPending{0};
        std::vector<std::mutex> mQueueMutexes;
        std::vector<std::thread> mWorkers;
        std::vector<std::deque<Task>> mQueues;

        void workerMainLoop(const std::size_t self) {
            std::mt19937 rng(std::random_device{}());

            while (!mShutdown.load(std::memory_order_acquire)) {
                Task task;
                if (tryPopOwn(self, task) || trySteal(self, task, rng)) {
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

        bool tryPopOwn(const std::size_t self, Task& task) {
            std::lock_guard<std::mutex> lock(mQueueMutexes.at(self));
            if (mQueues.at(self).empty()) {
                return false;
            }
            task = std::move(mQueues.at(self).front());
            mQueues.at(self).pop_front();
            return true;
        }

        bool trySteal(const std::size_t self, Task& task, std::mt19937& rng) {
            const auto scale = mQueues.size();
            if (scale <= 1) {
                return false;
            }

            std::uniform_int_distribution<std::size_t> dist(0, scale-1);
            for (std::size_t a = 0; a < scale; ++a) {
                const auto object = dist(rng);
                if (object == self) {
                    continue;
                }

                std::lock_guard<std::mutex> lock(mQueueMutexes.at(object));
                if (!mQueues.at(object).empty()) {
                    task = std::move(mQueues.at(object).front());
                    mQueues.at(object).pop_front();
                    return true;
                }
            }

            return false;
        }
    };
}

#endif //SIERRA_ODYPLAN_V2_CORE_RUNTIME_THREAD_WORKPOOL_H