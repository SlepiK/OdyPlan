#include <basic/window/count/count_window_buffer.h>
#include <cassert>

namespace sierra::odyplan::v3::basic::window::count {
    CountWindowBuffer::CountWindowBuffer(
        const CountWindowConfig& config,
        core::logical::port::Port mDataSchema,
        const core::runtime::types::RuntimeTypeRegistry& runtimeTypeRegistry)
        : mConfig(config), mRowLayout(std::move(mDataSchema), runtimeTypeRegistry) {}

    CountWindowBuffer::~CountWindowBuffer() {
        assert(mClosed && "CountWindowBuffer::close() was never called");
    }

    void CountWindowBuffer::begin() {
        mRowLayout.begin();
    }

    void CountWindowBuffer::push(const std::size_t index, const void* rawData) {
        mRowLayout.push(index, rawData);
    }

    void CountWindowBuffer::end(const Timestamp timestamp) {
        mRows.push_back(mRowLayout.pop());
        mTimestamps.push_back(timestamp);
        ++mTotalPushed;
    }

    void CountWindowBuffer::compute() {
        evict();
        mWindows.clear();

        while (mNextWindowStart + mConfig.size <= mTotalPushed) {
            mWindows.push_back(WindowRange{.start = mNextWindowStart, .count = mConfig.size});
            mNextWindowStart += mConfig.slide;
        }
    }

    void CountWindowBuffer::flush() {
        evict();
        mWindows.clear();

        if (const std::uint64_t remaining = mTotalPushed - mNextWindowStart; remaining > 0) {
            mWindows.push_back(WindowRange{.start = mNextWindowStart, .count = remaining});
            mNextWindowStart = mTotalPushed;
        }
    }

    std::size_t CountWindowBuffer::getWindowCount() const {
        return mWindows.size();
    }

    std::size_t CountWindowBuffer::getRowCountInWindow(const std::size_t index) const {
        return mWindows.at(index).count;
    }

    const std::byte* CountWindowBuffer::getData(const std::size_t windowIndex,
                                                const std::size_t rowIndex,
                                                const std::size_t fieldIndex) const {
        const auto& [start, count] = mWindows.at(windowIndex);
        const auto relIndex = start - mBaseIndex + rowIndex;
        return mRows.at(relIndex).data() + mRowLayout.offsetOf(fieldIndex);
    }

    void CountWindowBuffer::close() {
        for (auto& row : mRows) {
            mRowLayout.releaseRow(row);
        }

        mRows.clear();
        mTimestamps.clear();
        mClosed = true;
    }

    CountWindowBuffer::Timestamp CountWindowBuffer::getTimestamp(const std::size_t windowIndex,
                                                                 const std::size_t rowIndex) const {
        const auto& [start, count] = mWindows.at(windowIndex);
        const auto relIndex = start - mBaseIndex + rowIndex;
        return mTimestamps.at(relIndex);
    }

    void CountWindowBuffer::evict() {
        while (mBaseIndex < mNextWindowStart && !mRows.empty()) {
            mRowLayout.releaseRow(mRows.front());
            mRows.pop_front();
            mTimestamps.pop_front();
            ++mBaseIndex;
        }
    }
}  // namespace sierra::odyplan::v3::basic::window::count