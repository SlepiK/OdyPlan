#include <basic/window/time/time_window_buffer.h>

namespace sierra::odyplan::v3::basic::window::time {
    TimeWindowBuffer::TimeWindowBuffer(const TimeWindowConfig& config,
                                       core::logical::port::Port mDataSchema,
                                       const core::physical::types::ValueTypeRegistry& typeRegistry)
        : mConfig(config), mRowLayout(std::move(mDataSchema), typeRegistry) {}

    TimeWindowBuffer::~TimeWindowBuffer() {
        assert(mClosed && "TimeWindowBuffer::close() was never called");
    }

    void TimeWindowBuffer::begin() {
        mRowLayout.begin();
    }

    void TimeWindowBuffer::push(const std::size_t index, const void* rawData) {
        mRowLayout.push(index, rawData);
    }

    void TimeWindowBuffer::end(std::chrono::sys_time<std::chrono::milliseconds> timestamp) {
        if (!mWatermark || timestamp > *mWatermark) {
            mWatermark = timestamp;
        }

        if (mNextWindowStart && timestamp < mNextWindowStart.value()) {
            ++mDroppedLateRows;
            mRowLayout.discardCurrentRow();
            return;
        }

        insertAndSort(timestamp, mRowLayout.pop());
    }

    void TimeWindowBuffer::compute() {
        evict();
        mWindows.clear();

        if (mRows.empty() || !mWatermark) {
            return;
        }

        if (!mNextWindowStart) {
            mNextWindowStart = mRows.front().timestamp;
        }

        const auto watermark = *mWatermark - mConfig.allowedLateness;

        while (mNextWindowStart.has_value() &&
               mNextWindowStart.value() + mConfig.size <= watermark) {
            const auto current = mNextWindowStart.value();
            emit(current);
            mNextWindowStart = current + mConfig.slide;
        }
    }

    void TimeWindowBuffer::flush() {
        evict();
        mWindows.clear();

        if (mRows.empty() || !mWatermark) {
            return;
        }

        if (!mNextWindowStart) {
            mNextWindowStart = mRows.front().timestamp;
        }

        const auto lastTimestamp = mRows.back().timestamp;

        while (mNextWindowStart.has_value() && mNextWindowStart.value() <= lastTimestamp) {
            const auto current = mNextWindowStart.value();
            emit(current);
            mNextWindowStart = current + mConfig.slide;
        }
    }

    size_t TimeWindowBuffer::getWindowCount() const {
        return mWindows.size();
    }

    size_t TimeWindowBuffer::getRowCountInWindow(const std::size_t index) const {
        return mWindows.at(index).count;
    }

    const std::byte* TimeWindowBuffer::getData(const std::size_t windowIndex,
                                               const std::size_t rowIndex,
                                               const std::size_t fieldIndex) const {
        const auto& [start, count] = mWindows.at(windowIndex);
        const auto relIndex = start - mBaseIndex + rowIndex;
        return mRows.at(relIndex).values.data() + mRowLayout.offsetOf(fieldIndex);
    }

    void TimeWindowBuffer::close() {
        for (auto& [values, timestamp] : mRows) {
            mRowLayout.releaseRow(values);
        }

        mRows.clear();
        mClosed = true;
    }

    TimeWindowBuffer::Timestamp TimeWindowBuffer::getTimestamp(const std::size_t windowIndex,
                                                               const std::size_t rowIndex) const {
        const auto& [start, count] = mWindows.at(windowIndex);
        const auto relIndex = start - mBaseIndex + rowIndex;
        return mRows.at(relIndex).timestamp;
    }

    void TimeWindowBuffer::evict() {
        if (!mNextWindowStart) {
            return;
        }

        while (!mRows.empty() && mRows.front().timestamp < mNextWindowStart.value()) {
            mRowLayout.releaseRow(mRows.front().values);
            mRows.pop_front();
            ++mBaseIndex;
        }
    }

    void TimeWindowBuffer::emit(const Timestamp start) {
        const auto end = start + mConfig.size;
        const auto iterStart = std::ranges::lower_bound(mRows, start, {}, &Row::timestamp);
        const auto iterEnd = std::ranges::lower_bound(mRows, end, {}, &Row::timestamp);

        if (const auto count = static_cast<std::uint64_t>(std::distance(iterStart, iterEnd));
            count > 0) {
            const auto startIndex =
                static_cast<std::uint64_t>(std::distance(mRows.begin(), iterStart)) + mBaseIndex;
            mWindows.push_back(WindowRange{.start = startIndex, .count = count});
        }
    }

    void TimeWindowBuffer::insertAndSort(const Timestamp timestamp,
                                         std::vector<std::byte>&& values) {
        const auto position = std::ranges::upper_bound(mRows, timestamp, {}, &Row::timestamp);
        mRows.insert(position, Row{.values = std::move(values), .timestamp = timestamp});
    }
}  // namespace sierra::odyplan::v3::basic::window::time