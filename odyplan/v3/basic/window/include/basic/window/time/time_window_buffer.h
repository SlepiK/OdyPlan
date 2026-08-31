#ifndef SIERRA_ODYPLAN_V2_COMMON_WINDOW_TIME_WINDOW_BUFFER_H
#define SIERRA_ODYPLAN_V2_COMMON_WINDOW_TIME_WINDOW_BUFFER_H

#include <basic/window/count/count_window_config.h>
#include <basic/window/window_row_layout.h>
#include <deque>
#include <map>
#include <vector>

namespace sierra::odyplan::v3::basic::window::time {
    class ITimeWindowBuffer {
    public:
        ITimeWindowBuffer() = default;

        virtual ~ITimeWindowBuffer() = default;

        ITimeWindowBuffer(const ITimeWindowBuffer&) = delete;
        ITimeWindowBuffer& operator=(const ITimeWindowBuffer&) = delete;
        ITimeWindowBuffer(ITimeWindowBuffer&&) = delete;
        ITimeWindowBuffer& operator=(ITimeWindowBuffer&&) = delete;

        virtual void begin() = 0;

        virtual void push(std::size_t index, const void* rawData) = 0;

        virtual void end(std::int64_t eventTs, std::int64_t windowStart, std::int64_t calculatedWindowEnd) = 0;

        virtual void drain(std::int64_t threshold, bool isFinal) = 0;

        virtual void close() = 0;

        [[nodiscard]] virtual std::size_t getDrainedRowCount() const = 0;

        [[nodiscard]] virtual const std::byte* getDrainedData(std::size_t rowIndex, std::size_t fieldIndex) const = 0;

        [[nodiscard]] virtual std::int64_t getDrainedWindowStart(std::size_t rowIndex) const = 0;

        [[nodiscard]] virtual std::int64_t getDrainedWindowEnd(std::size_t rowIndex) const = 0;

        [[nodiscard]] virtual std::int64_t getDrainedTimestamp(std::size_t rowIndex) const = 0;

        [[nodiscard]] virtual bool getDrainedIsLastInWindow(std::size_t rowIndex) const = 0;

        [[nodiscard]] virtual bool getDrainedIsClosedByFlush(std::size_t rowIndex) const = 0;
    };

    class TimeWindowBuffer : public ITimeWindowBuffer {
    public:
        TimeWindowBuffer(core::logical::port::Port schema, const core::runtime::types::RuntimeTypeRegistry& runtimeTypeRegistry);

        ~TimeWindowBuffer() override;

        TimeWindowBuffer(const TimeWindowBuffer&) = delete;
        TimeWindowBuffer& operator=(const TimeWindowBuffer&) = delete;
        TimeWindowBuffer(TimeWindowBuffer&&) = delete;
        TimeWindowBuffer& operator=(TimeWindowBuffer&&) = delete;

        void begin() override;

        void push(std::size_t index, const void* rawData) override;

        void end(std::int64_t eventTs, std::int64_t windowStart, std::int64_t calculatedWindowEnd) override;

        void drain(std::int64_t threshold, bool isFinal) override;

        void close() override;

        [[nodiscard]] std::size_t getDrainedRowCount() const override;

        [[nodiscard]] const std::byte* getDrainedData(std::size_t rowIndex, std::size_t fieldIndex) const override;

        [[nodiscard]] std::int64_t getDrainedWindowStart(std::size_t rowIndex) const override;

        [[nodiscard]] std::int64_t getDrainedWindowEnd(std::size_t rowIndex) const override;

        [[nodiscard]] std::int64_t getDrainedTimestamp(std::size_t rowIndex) const override;

        [[nodiscard]] bool getDrainedIsLastInWindow(std::size_t rowIndex) const override;

        [[nodiscard]] bool getDrainedIsClosedByFlush(std::size_t rowIndex) const override;

    private:
        struct WindowBucket {
            std::deque<std::int64_t> rowStarts;
            std::deque<std::vector<std::byte>> rows;
            std::deque<std::int64_t> timestamps;
        };

        struct DrainedRow {
            std::int64_t windowStartTs{0};
            std::int64_t windowEndTs{0};
            std::int64_t ts{0};
            bool isLast{false};
            bool isClosedByFlush{false};
            std::vector<std::byte> data;
        };

        WindowRowLayout mRowLayout;
        std::map<std::int64_t, WindowBucket> mOpenWindows;
        std::vector<DrainedRow> mDrainedRows;
        bool mClosed = false;
    };

}  // namespace sierra::odyplan::v3::basic::window::count

#endif  // SIERRA_ODYPLAN_V2_COMMON_WINDOW_TIME_WINDOW_BUFFER_H