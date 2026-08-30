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
        const std::uint64_t step = (mConfig.type == WindowType::ADVANCE) ? mConfig.advance : mConfig.slide;

        while (mNextWindowStart + mConfig.size + 1 <= mTotalPushed) {
            mWindows.push_back(CountWindowRange{.start = mNextWindowStart, .count = mConfig.size});
            mNextWindowStart += step;
        }
    }

    void CountWindowBuffer::flush() {
        evict();
        mWindows.clear();
        const std::uint64_t step = (mConfig.type == WindowType::ADVANCE) ? mConfig.advance : mConfig.slide;

        while (mNextWindowStart < mTotalPushed) {
            const std::uint64_t remaining = mTotalPushed - mNextWindowStart;
            const std::uint64_t count = std::min<std::uint64_t>(mConfig.size, remaining);
            mWindows.push_back(CountWindowRange{.start = mNextWindowStart, .count = count});
            mNextWindowStart += step;
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

        if (mBaseIndex > 0) {
            const auto shift = mBaseIndex;
            mBaseIndex = 0;
            mNextWindowStart -= shift;
            mTotalPushed -= shift;
        }
    }

    CountWindowBuffer::Timestamp CountWindowBuffer::getWindowStart(const std::size_t windowIndex) const {
        const auto& [start, count] = mWindows.at(windowIndex);
        const auto relIndex = start - mBaseIndex;
        return mTimestamps.at(relIndex);
    }

    CountWindowBuffer::Timestamp CountWindowBuffer::getWindowStartRow(const std::size_t windowIndex, const std::size_t rowIndex) const {
        const auto& [start, count] = mWindows.at(windowIndex);
        const std::size_t effRow = (mConfig.type == WindowType::ADVANCE) ? rowIndex : 0;
        const auto relIndex = start - mBaseIndex + effRow;
        return mTimestamps.at(relIndex);
    }

    std::optional<CountWindowBuffer::Timestamp> CountWindowBuffer::getWindowEnd(const std::size_t windowIndex) const {
        const auto& [start, count] = mWindows.at(windowIndex);
        const auto lastRelIndex = start - mBaseIndex + count;

        if (lastRelIndex < mTimestamps.size()) {
            return mTimestamps.at(lastRelIndex);
        }

        //return std::nullopt;

        const auto lastRowInWindowRelIndex = start - mBaseIndex + count - 1;
        return mTimestamps.at(lastRowInWindowRelIndex);
    }

    bool CountWindowBuffer::isFlushedClosed(const std::size_t windowIndex) const {
        return mWindows.at(windowIndex).count < mConfig.size;
    }
}  // namespace sierra::odyplan::v3::basic::window::count