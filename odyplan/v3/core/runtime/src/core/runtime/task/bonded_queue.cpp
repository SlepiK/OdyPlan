#include <core/runtime/task/bonded_queue.h>

namespace sierra::odyplan::v3::core::runtime::task {
    BoundedQueue::BoundedQueue(const std::size_t maxSize) : mMaxSize(maxSize) {
    }

    void BoundedQueue::push(void* item) {
        std::unique_lock<std::mutex> lock(mMutex);
        mNotFull.wait(lock, [this] { return mQueue.size() < mMaxSize || mClosed; });
        if (mClosed) {
            return;
        }
        mQueue.push(item);
        lock.unlock();
        mNotEmpty.notify_one();
    }

    std::optional<void*> BoundedQueue::pop() {
        std::unique_lock<std::mutex> lock(mMutex);
        mNotEmpty.wait(lock, [this] { return !mQueue.empty() || mClosed; });
        if (mQueue.empty()) {
            return std::nullopt;
        }
        void* item = mQueue.front();
        mQueue.pop();
        lock.unlock();
        mNotFull.notify_one();
        return item;
    }

    void BoundedQueue::close() {
        {
            const std::scoped_lock lock(mMutex);
            mClosed = true;
        }
        mNotEmpty.notify_all();
        mNotFull.notify_all();
    }
}