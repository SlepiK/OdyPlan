#ifndef SIERRA_ODYPLAN_V2_COMMON_WINDOW_COUNT_WINDOW_BUFFER_H
#define SIERRA_ODYPLAN_V2_COMMON_WINDOW_COUNT_WINDOW_BUFFER_H

#include <basic/window/count/count_window_config.h>
#include <basic/window/count/count_window_range.h>
#include <basic/window/window_row_layout.h>
#include <deque>
#include <vector>

namespace sierra::odyplan::v3::basic::window::count {
    class ICountWindowBuffer {
    public:
        using Timestamp = std::chrono::sys_time<std::chrono::milliseconds>;

        ICountWindowBuffer() = default;
        virtual ~ICountWindowBuffer() = default;
        ICountWindowBuffer(ICountWindowBuffer const&) = delete;
        ICountWindowBuffer& operator=(ICountWindowBuffer const&) = delete;
        ICountWindowBuffer(ICountWindowBuffer&&) = delete;
        ICountWindowBuffer& operator=(ICountWindowBuffer&&) = delete;

        virtual void begin() = 0;

        virtual void close() = 0;

        virtual void push(std::size_t index, const void* rawData) = 0;

        virtual void end(std::chrono::sys_time<std::chrono::milliseconds> timestamp) = 0;

        virtual void compute() = 0;

        virtual void flush() = 0;

        [[nodiscard]] virtual std::size_t getWindowCount() const = 0;

        [[nodiscard]] virtual std::size_t getRowCountInWindow(std::size_t index) const = 0;

        [[nodiscard]] virtual const std::byte*
        getData(std::size_t windowIndex, std::size_t rowIndex, std::size_t fieldIndex) const = 0;

        [[nodiscard]] virtual std::chrono::sys_time<std::chrono::milliseconds>
        getTimestamp(std::size_t windowIndex, std::size_t rowIndex) const = 0;

        [[nodiscard]] virtual std::chrono::sys_time<std::chrono::milliseconds>
        getWindowStart(std::size_t windowIndex) const = 0;

        [[nodiscard]] virtual std::chrono::sys_time<std::chrono::milliseconds>
        getWindowStartRow(std::size_t windowIndex, std::size_t rowIndex) const = 0;

        [[nodiscard]] virtual std::optional<std::chrono::sys_time<std::chrono::milliseconds>>
        getWindowEnd(std::size_t windowIndex) const = 0;

        [[nodiscard]] virtual bool isFlushedClosed(std::size_t windowIndex) const = 0;
    };

    class CountWindowBuffer final : public ICountWindowBuffer {
    public:
        using Timestamp = std::chrono::sys_time<std::chrono::milliseconds>;

        CountWindowBuffer(const CountWindowConfig& config,
                          core::logical::port::Port mDataSchema,
                          const core::runtime::types::RuntimeTypeRegistry& runtimeTypeRegistry);

        ~CountWindowBuffer() override;

        CountWindowBuffer(CountWindowBuffer const&) = delete;
        CountWindowBuffer& operator=(CountWindowBuffer const&) = delete;
        CountWindowBuffer(CountWindowBuffer&&) = delete;
        CountWindowBuffer& operator=(CountWindowBuffer&&) = delete;

        void begin() override;

        void push(std::size_t index, const void* rawData) override;

        void end(Timestamp timestamp) override;

        void compute() override;

        void flush() override;

        [[nodiscard]] std::size_t getWindowCount() const override;

        [[nodiscard]] std::size_t getRowCountInWindow(std::size_t index) const override;

        [[nodiscard]] const std::byte* getData(std::size_t windowIndex,
                                               std::size_t rowIndex,
                                               std::size_t fieldIndex) const override;

        void close() override;

        [[nodiscard]] Timestamp getTimestamp(std::size_t windowIndex,
                                             std::size_t rowIndex) const override;

        [[nodiscard]] Timestamp getWindowStart(std::size_t windowIndex) const override;

        [[nodiscard]] Timestamp getWindowStartRow(std::size_t windowIndex, std::size_t rowIndex) const override;

        [[nodiscard]] std::optional<Timestamp> getWindowEnd(std::size_t windowIndex) const override;

        [[nodiscard]] bool isFlushedClosed(std::size_t windowIndex) const override;
    private:
        CountWindowConfig mConfig;
        WindowRowLayout mRowLayout;

        std::deque<std::vector<std::byte>> mRows;
        std::deque<Timestamp> mTimestamps;
        std::vector<CountWindowRange> mWindows;

        std::uint64_t mBaseIndex = 0;
        std::uint64_t mTotalPushed = 0;
        std::uint64_t mNextWindowStart = 0;
        bool mClosed = false;

        void evict();
    };
}  // namespace sierra::odyplan::v3::basic::window::count

#endif  // SIERRA_ODYPLAN_V2_COMMON_WINDOW_COUNT_WINDOW_BUFFER_H