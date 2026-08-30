#include <assert.h>
#include <ranges>
#include <basic/window/time/time_window_buffer.h>

namespace sierra::odyplan::v3::basic::window::time {
    TimeWindowBuffer::TimeWindowBuffer(core::logical::port::Port schema,
                                       const core::runtime::types::RuntimeTypeRegistry &runtimeTypeRegistry)
                                           : mRowLayout(std::move(schema), runtimeTypeRegistry) {
    }

    TimeWindowBuffer::~TimeWindowBuffer() {
        assert(mClosed && "TimeWindowBuffer::close() not called!");
    }

    void TimeWindowBuffer::begin() {
        mRowLayout.begin();
    }

    void TimeWindowBuffer::push(const std::size_t index, const void *rawData) {
        mRowLayout.push(index, rawData);
    }

    void TimeWindowBuffer::end(const std::int64_t eventTs, const std::int64_t windowStart, const std::int64_t calculatedWindowEnd) {
        auto row = mRowLayout.pop();
        auto& windowBucket = mOpenWindows[calculatedWindowEnd];
        windowBucket.rowStarts.push_back(windowStart);
        windowBucket.rows.push_back(std::move(row));
        windowBucket.timestamps.push_back(eventTs);
    }

    void TimeWindowBuffer::drain(const std::int64_t threshold, const bool isFinal) {
        for (auto& drainedRow : mDrainedRows) {
            mRowLayout.releaseRow(drainedRow.data);
        }
        mDrainedRows.clear();

        //threshold = watermark (punctuation) - allowedLateness
        const auto endIter = isFinal ? mOpenWindows.end() : mOpenWindows.upper_bound(threshold);

        for (auto iter = mOpenWindows.begin(); iter != endIter;) {
            auto& windowBucket = iter->second;
            while (!windowBucket.rows.empty()) {
                //is last entry in the bucket?
                const bool isLast = (windowBucket.rows.size() == 1);
                mDrainedRows.push_back(DrainedRow{
                    .windowStartTs = windowBucket.rowStarts.front(),
                    .windowEndTs = iter->first,
                    .ts = windowBucket.timestamps.front(),
                    .isLast = isLast,
                    .isClosedByFlush = isFinal,
                    .data = std::move(windowBucket.rows.front()),
                });
                windowBucket.rows.pop_front();
                windowBucket.timestamps.pop_front();
                windowBucket.rowStarts.pop_front();
            }
            iter = mOpenWindows.erase(iter);
        }
    }

    void TimeWindowBuffer::close() {
        for (auto& drainedRow : mDrainedRows) {
            mRowLayout.releaseRow(drainedRow.data);
        }
        mDrainedRows.clear();

        for (auto &windowBucket: mOpenWindows | std::views::values) {
            for (auto& row: windowBucket.rows) {
                mRowLayout.releaseRow(row);
            }
        }
        mOpenWindows.clear();
        mClosed = true;
    }

    std::size_t TimeWindowBuffer::getDrainedRowCount() const {
        return mDrainedRows.size();
    }

    const std::byte *TimeWindowBuffer::getDrainedData(const std::size_t rowIndex, const std::size_t fieldIndex) const {
        return mDrainedRows.at(rowIndex).data.data() + mRowLayout.offsetOf(fieldIndex);
    }

    std::int64_t TimeWindowBuffer::getDrainedWindowStart(const std::size_t rowIndex) const {
        return mDrainedRows.at(rowIndex).windowStartTs;
    }

    std::int64_t TimeWindowBuffer::getDrainedWindowEnd(const std::size_t rowIndex) const {
        return mDrainedRows.at(rowIndex).windowEndTs;
    }

    std::int64_t TimeWindowBuffer::getDrainedTimestamp(const std::size_t rowIndex) const {
        return mDrainedRows.at(rowIndex).ts;
    }

    bool TimeWindowBuffer::getDrainedIsLastInWindow(const std::size_t rowIndex) const {
        return mDrainedRows.at(rowIndex).isLast;
    }

    bool TimeWindowBuffer::getDrainedIsClosedByFlush(const std::size_t rowIndex) const {
        return mDrainedRows.at(rowIndex).isClosedByFlush;
    }
}
