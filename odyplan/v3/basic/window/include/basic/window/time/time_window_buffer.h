#ifndef SIERRA_ODYPLAN_V2_COMMON_WINDOW_TIME_WINDOWBUFFER_H
#define SIERRA_ODYPLAN_V2_COMMON_WINDOW_TIME_WINDOWBUFFER_H

#include <deque>
#include <algorithm>
#include <basic/window/window_buffer.h>
#include <basic/window/time/time_window_config.h>
#include <basic/window/window_row_layout.h>
#include <basic/window/window_range.h>

namespace sierra::odyplan::v3::basic::window::time {
    class TimeWindowBuffer final : public IWindowBuffer {
    public:
        using Timestamp = std::chrono::sys_time<std::chrono::milliseconds>;

        TimeWindowBuffer(const TimeWindowConfig &config, core::logical::port::Port mDataSchema, const core::physical::types::ValueTypeRegistry& typeRegistry);

        ~TimeWindowBuffer() override;

        TimeWindowBuffer(TimeWindowBuffer const&) = delete;
        TimeWindowBuffer& operator =(TimeWindowBuffer const&) = delete;
        TimeWindowBuffer(TimeWindowBuffer&&) = delete;
        TimeWindowBuffer& operator=(TimeWindowBuffer&&) = delete;

        void begin() override;

        void push(std::size_t index, const void *rawData) override;

        void end(std::chrono::sys_time<std::chrono::milliseconds> timestamp) override;

        void compute() override;

        void flush() override;

        [[nodiscard]] size_t getWindowCount() const override;

        [[nodiscard]] size_t getRowCountInWindow(std::size_t index) const override;

        [[nodiscard]] const std::byte* getData(std::size_t windowIndex, std::size_t rowIndex, std::size_t fieldIndex) const override;

        void close() override;

        [[nodiscard]] Timestamp getTimestamp(std::size_t windowIndex, std::size_t rowIndex) const override;

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

        void evict();

        void emit(Timestamp start);

        void insertAndSort(Timestamp timestamp, std::vector<std::byte>&& values);
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_WINDOW_TIME_WINDOWBUFFER_H