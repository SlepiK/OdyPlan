#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_RUNTIME_COUNT_WINDOW_BUFFER_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_RUNTIME_COUNT_WINDOW_BUFFER_HPP

#include <basic/operators/pipe/window/details/count_window_options.h>
#include <basic/operators/pipe/window/runtime/count_window_range.h>
#include <basic/storage/row_layout.h>
#include <deque>
#include <optional>
#include <vector>

namespace sierra::odyplan::v3::basic::operators::pipe::window::runtime {
    class ICountWindowBuffer {
    public:
        using Timestamp = std::int64_t;

        ICountWindowBuffer() = default;

        virtual ~ICountWindowBuffer() = default;

        ICountWindowBuffer(ICountWindowBuffer const&) = delete;
        ICountWindowBuffer& operator=(ICountWindowBuffer const&) = delete;
        ICountWindowBuffer(ICountWindowBuffer&&) = delete;
        ICountWindowBuffer& operator=(ICountWindowBuffer&&) = delete;

        virtual void begin() = 0;
        virtual void push(std::uint64_t fieldIndex, const void* rawData) = 0;
        virtual void end(Timestamp startTs, Timestamp endTs) = 0;

        [[nodiscard]] virtual bool ready() const = 0;

        [[nodiscard]] virtual std::uint64_t getRowCount() const = 0;
        [[nodiscard]] virtual const std::byte* getData(std::uint64_t rowIndex, std::uint64_t fieldIndex) const = 0;
        [[nodiscard]] virtual Timestamp getRowStart(std::uint64_t rowIndex) const = 0;
        [[nodiscard]] virtual Timestamp getWindowEnd() const = 0;

        virtual void advanceWindow() = 0;

        virtual void close() = 0;

        [[nodiscard]] virtual std::uint64_t getEmitCount() const = 0;
    };

    class CountWindowBuffer final : public ICountWindowBuffer {
    public:
        CountWindowBuffer(const details::CountWindowOptions& config,
                          core::logical::port::Port mDataSchema,
                          const core::runtime::types::RuntimeTypeRegistry& runtimeTypeRegistry);

        ~CountWindowBuffer() override;

        CountWindowBuffer(CountWindowBuffer const&) = delete;
        CountWindowBuffer& operator=(CountWindowBuffer const&) = delete;
        CountWindowBuffer(CountWindowBuffer&&) = delete;
        CountWindowBuffer& operator=(CountWindowBuffer&&) = delete;


        void begin() override;
        void push(std::uint64_t fieldIndex, const void* rawData) override;
        void end(Timestamp startTs, Timestamp endTs) override;

        [[nodiscard]] bool ready() const override;

        [[nodiscard]] std::uint64_t getRowCount() const override;
        [[nodiscard]] const std::byte* getData(std::uint64_t rowIndex, std::uint64_t fieldIndex) const override;
        [[nodiscard]] Timestamp getRowStart(std::uint64_t rowIndex) const override;
        [[nodiscard]] Timestamp getWindowEnd() const override;

        void advanceWindow() override;

        void close() override;

        [[nodiscard]] std::uint64_t getEmitCount() const override;
    private:
        struct Row {
            std::vector<std::byte> data;
            Timestamp startTs;
            Timestamp endTs;
        };

        details::CountWindowOptions mConfig;
        storage::RowLayout mRowLayout;
        core::logical::port::Port mSchema;

        std::deque<Row> mRowBuffer;
        std::optional<Timestamp> mFirstElementStart;
        Row mPendingRow;

        bool mReady = false;
        std::uint64_t mReadyRowCount = 0;
        std::uint64_t mReadyEmitCount = 0;
        std::int64_t mReadyEnd = 0;
        bool mFlushed = false;
    };
}

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_RUNTIME_COUNT_WINDOW_BUFFER_HPP