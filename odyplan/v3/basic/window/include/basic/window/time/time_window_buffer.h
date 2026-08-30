#ifndef SIERRA_ODYPLAN_V2_COMMON_WINDOW_TIME_WINDOW_BUFFER_H
#define SIERRA_ODYPLAN_V2_COMMON_WINDOW_TIME_WINDOW_BUFFER_H

#include <basic/window/count/count_window_config.h>
#include <basic/window/window_buffer.h>
#include <basic/window/window_range.h>
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

        virtual void end(std::uint64_t eventTs, std::uint64_t windowStart, std::uint64_t calculatedWindowEnd) = 0;

        virtual void drain(std::uint64_t threshold, bool isFinal) = 0;

        virtual void close() = 0;

        [[nodiscard]] virtual std::size_t getDrainedRowCount() const = 0;

        [[nodiscard]] virtual const std::byte* getDrainedData(std::size_t rowIndex, std::size_t fieldIndex) const = 0;

        [[nodiscard]] virtual std::uint64_t getDrainedWindowStart(std::size_t rowIndex) const = 0;

        [[nodiscard]] virtual std::uint64_t getDrainedWindowEnd(std::size_t rowIndex) const = 0;

        [[nodiscard]] virtual std::uint64_t getDrainedTimestamp(std::size_t rowIndex) const = 0;
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

        void end(std::uint64_t eventTs, std::uint64_t windowStart, std::uint64_t calculatedWindowEnd) override;

        void drain(std::uint64_t threshold, bool isFinal) override;

        void close() override;

        [[nodiscard]] std::size_t getDrainedRowCount() const override;

        [[nodiscard]] const std::byte* getDrainedData(std::size_t rowIndex, std::size_t fieldIndex) const override;

        [[nodiscard]] std::uint64_t getDrainedWindowStart(std::size_t rowIndex) const override;

        [[nodiscard]] std::uint64_t getDrainedWindowEnd(std::size_t rowIndex) const override;

        [[nodiscard]] std::uint64_t getDrainedTimestamp(std::size_t rowIndex) const override;

    private:
        struct WindowBucket {
            std::uint64_t windowStart{0};
            std::deque<std::vector<std::byte>> rows;
            std::deque<std::uint64_t> timestamps;
        };

        struct DrainedRow {
            std::uint64_t windowStartTs{0};
            std::uint64_t windowEndTs{0};
            std::uint64_t ts{0};
            std::vector<std::byte> data;
        };

        WindowRowLayout mRowLayout;
        std::map<std::uint64_t, WindowBucket> mOpenWindows;
        std::vector<DrainedRow> mDrainedRows;
        bool mClosed = false;
    };

}  // namespace sierra::odyplan::v3::basic::window::count

#endif  // SIERRA_ODYPLAN_V2_COMMON_WINDOW_TIME_WINDOW_BUFFER_H