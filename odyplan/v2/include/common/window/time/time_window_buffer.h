#ifndef SIERRA_ODYPLAN_V2_COMMON_WINDOW_TIME_WINDOWBUFFER_H
#define SIERRA_ODYPLAN_V2_COMMON_WINDOW_TIME_WINDOWBUFFER_H

#include <deque>
#include <algorithm>
#include <common/window/window_buffer.h>
#include <common/window/time/time_window_config.h>
#include <common/window/window_row_layout.h>

namespace sierra::odyplan::v2::common::window::time {
    class TimeWindowBuffer final : public IWindowBuffer {
    public:
        using Timestamp = std::chrono::sys_time<std::chrono::milliseconds>;

        TimeWindowBuffer(const TimeWindowConfig &config, core::logical::port::Port mDataSchema, const core::common::type::TypeRegistry& typeRegistry)
            : mConfig(config),
                mRowLayout(std::move(mDataSchema), typeRegistry) {
        }

        ~TimeWindowBuffer() override {
            assert(mClosed && "TimeWindowBuffer::close() was never called");
        }

        TimeWindowBuffer(TimeWindowBuffer const&) = delete;
        TimeWindowBuffer& operator =(TimeWindowBuffer const&) = delete;
        TimeWindowBuffer(TimeWindowBuffer&&) = delete;
        TimeWindowBuffer& operator=(TimeWindowBuffer&&) = delete;

        void begin() override {
            mRowLayout.begin();
        }

        void push(const std::size_t index, const void *rawData) override {
            mRowLayout.push(index, rawData);
        }

        void end(std::chrono::sys_time<std::chrono::milliseconds> timestamp) override {
            if (!mWatermark || timestamp > *mWatermark) {
                mWatermark = timestamp;
            }

            if (mNextWindowStart && timestamp < *mNextWindowStart) {
                ++mDroppedLateRows;
                mRowLayout.discardCurrentRow();
                return;
            }

            insertAndSort(timestamp, mRowLayout.pop());

        }

        void compute() override {
            evict();
            mWindows.clear();

            if (mRows.empty() || !mWatermark) {
                return;
            }

            if (!mNextWindowStart) {
                mNextWindowStart = mRows.front().timestamp;
            }

            const auto watermark = *mWatermark - mConfig.allowedLateness;

            while (*mNextWindowStart + mConfig.size <= watermark) {
                emit(*mNextWindowStart);
                *mNextWindowStart += mConfig.slide;
            }
        }

        void flush() override {
            evict();
            mWindows.clear();

            if (mRows.empty() || !mWatermark) {
                return;
            }

            if (!mNextWindowStart) {
                mNextWindowStart = mRows.front().timestamp;
            }

            const auto lastTimestamp = mRows.back().timestamp;

            while (*mNextWindowStart <= lastTimestamp) {
                emit(*mNextWindowStart);
                *mNextWindowStart += mConfig.slide;
            }

        }

        [[nodiscard]] size_t getWindowCount() const override {
            return mWindows.size();
        }

        [[nodiscard]] size_t getRowCountInWindow(const std::size_t index) const override {
            return mWindows.at(index).count;
        }

        [[nodiscard]] const std::byte* getData(const std::size_t windowIndex, const std::size_t rowIndex, const std::size_t fieldIndex) const override {
            const auto& range = mWindows.at(windowIndex);
            const auto relIndex = range.start - mBaseIndex + rowIndex;
            return mRows.at(relIndex).values.data() + mRowLayout.offsetOf(fieldIndex);
        }

        void close() override {
            for (auto& row : mRows) {
                mRowLayout.releaseRow(row.values);
            }

            mRows.clear();
            mClosed = true;
        }

    private:
        struct Row {
            std::vector<std::byte> values;
            Timestamp timestamp;
        };

        TimeWindowConfig mConfig;
        WindowRowLayout mRowLayout;

        std::deque<Row> mRows;
        std::vector<WindowRange> mWindows;

        std::uint64_t mBaseIndex = 0;
        std::optional<Timestamp> mWatermark;
        std::optional<Timestamp> mNextWindowStart;
        std::uint64_t mDroppedLateRows = 0;
        bool mClosed = false;

        void evict() {
            if (!mNextWindowStart) {
                return;
            }

            while (!mRows.empty() && mRows.front().timestamp < *mNextWindowStart) {
                mRowLayout.releaseRow(mRows.front().values);
                mRows.pop_front();
                ++mBaseIndex;
            }
        }

        void emit(const Timestamp start) {
            const auto end = start + mConfig.size;
            const auto iterStart = std::ranges::lower_bound(mRows, start, {}, &Row::timestamp);
            const auto iterEnd = std::ranges::lower_bound(mRows, end, {}, &Row::timestamp);

            if (const auto count = static_cast<std::uint64_t>(std::distance(iterStart, iterEnd)); count > 0) {
                const auto startIndex = static_cast<std::uint64_t>(std::distance(mRows.begin(), iterStart)) + mBaseIndex;
                mWindows.push_back(WindowRange{.start=startIndex, .count=count});
            }
        }

        void insertAndSort(const Timestamp timestamp, std::vector<std::byte>&& values) {
            const auto position = std::ranges::upper_bound(mRows, timestamp, {}, &Row::timestamp);
            mRows.insert(position, Row{.values=std::move(values), .timestamp=timestamp});
        }
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_WINDOW_TIME_WINDOWBUFFER_H