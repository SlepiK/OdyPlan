#ifndef SIERRA_ODYPLAN_V3_CORE_INTERNAL_THREADS_COMPUTE_THREAD_POOL_HPP
#define SIERRA_ODYPLAN_V3_CORE_INTERNAL_THREADS_COMPUTE_THREAD_POOL_HPP

#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <future>
#include <deque>
#include <cstddef>
#include <random>

#include <core/internal/tasks/task.h>

namespace sierra::odyplan::v3::core::internal::threads {
	class ComputeThreadPool {
	public:
		using Task = tasks::Task;

		explicit ComputeThreadPool(std::size_t poolSize = std::max<std::size_t>(1, std::thread::hardware_concurrency()));

		~ComputeThreadPool();

		ComputeThreadPool(const ComputeThreadPool&) = delete;
        ComputeThreadPool& operator=(const ComputeThreadPool&) = delete;
        ComputeThreadPool(ComputeThreadPool&&) = delete;
        ComputeThreadPool& operator=(ComputeThreadPool&&) = delete;

		std::future<void> submitTask(Task task);

	private:
        std::mutex mWakeMutex;
        std::condition_variable mWakeCV;
        std::atomic_bool mShutdown{false};
        std::atomic<std::size_t> mRoundRobin{0};
        std::atomic<std::size_t> mPending{0};
        std::vector<std::mutex> mQueueMutexes;
        std::vector<std::thread> mWorkers;
        std::vector<std::deque<Task>> mQueues;

		void workerLoop(std::size_t workerId);

		bool tryPopOwn(std::size_t workerId, Task& task);

		bool trySteal(std::size_t workerId, Task& task, std::mt19937& rng);
	};
}

#endif //SIERRA_ODYPLAN_V3_CORE_INTERNAL_THREADS_COMPUTE_THREAD_POOL_HPP