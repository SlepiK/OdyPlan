#ifndef SIERRA_ODYPLAN_V2_COMMON_WINDOW_COUNT_WINDOWBUFFER_H
#define SIERRA_ODYPLAN_V2_COMMON_WINDOW_COUNT_WINDOWBUFFER_H

#include <deque>
#include <vector>
#include <basic/window/window_buffer.h>
#include <basic/window/window_row_layout.h>
#include <basic/window/count/count_window_config.h>
#include <basic/window/window_range.h>

namespace sierra::odyplan::v3::basic::window::count {
    class CountWindowBuffer final : public IWindowBuffer {
    public:
        using Timestamp = std::chrono::sys_time<std::chrono::milliseconds>;

        CountWindowBuffer(const CountWindowConfig &config, core::logical::port::Port mDataSchema, const core::physical::types::ValueTypeRegistry& typeRegistry);

        ~CountWindowBuffer() override;

        CountWindowBuffer(CountWindowBuffer const&) = delete;
        CountWindowBuffer& operator =(CountWindowBuffer const&) = delete;
        CountWindowBuffer(CountWindowBuffer&&) = delete;
        CountWindowBuffer& operator=(CountWindowBuffer&&) = delete;

        void begin() override;

        void push(std::size_t index, const void *rawData) override;

        void end(Timestamp timestamp) override;

        void compute() override;

        void flush() override;

        [[nodiscard]] std::size_t getWindowCount() const override;

        [[nodiscard]] std::size_t getRowCountInWindow(std::size_t index) const override;

        [[nodiscard]] const std::byte* getData(std::size_t windowIndex, std::size_t rowIndex, std::size_t fieldIndex) const override;

        void close() override;

        [[nodiscard]] Timestamp getTimestamp(std::size_t windowIndex, std::size_t rowIndex) const override;

    private:
        CountWindowConfig mConfig;
        WindowRowLayout mRowLayout;

        std::deque<std::vector<std::byte>> mRows;
        std::deque<Timestamp> mTimestamps;
        std::vector<WindowRange> mWindows;

        std::uint64_t mBaseIndex = 0;
        std::uint64_t mTotalPushed = 0;
        std::uint64_t mNextWindowStart = 0;
        bool mClosed = false;

        void evict();
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_WINDOW_COUNT_WINDOWBUFFER_H