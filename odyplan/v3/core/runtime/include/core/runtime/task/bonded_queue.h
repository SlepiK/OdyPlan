#ifndef SIERRA_ODYPLAN_V3_CORE_RUNTIME_TASK_BONDED_QUEUE_HPP
#define SIERRA_ODYPLAN_V3_CORE_RUNTIME_TASK_BONDED_QUEUE_HPP
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <optional>
#include <queue>

namespace sierra::odyplan::v3::core::runtime::task {
    class BoundedQueue {
    public:
        explicit BoundedQueue(const std::size_t maxSize = 8192);

        void push(void* item);

        std::optional<void*> pop();

        void close();

    private:
        std::mutex mMutex;
        std::condition_variable mNotEmpty;
        std::condition_variable mNotFull;
        std::queue<void*> mQueue;
        std::size_t mMaxSize;
        bool mClosed = false;
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_RUNTIME_TASK_BONDED_QUEUE_HPP