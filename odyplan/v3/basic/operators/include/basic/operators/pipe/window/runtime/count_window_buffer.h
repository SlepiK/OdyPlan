#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_RUNTIME_COUNT_WINDOW_BUFFER_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_RUNTIME_COUNT_WINDOW_BUFFER_HPP

#include <basic/operators/pipe/window/details/count_window_options.h>
#include <basic/operators/pipe/window/runtime/count_window_range.h>
#include <basic/storage/storage/row_layout.h>
#include <deque>
#include <optional>
#include <vector>
#include <core/shared/types/global_types.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::runtime {
    class ICountWindowBuffer {
    public:
        ICountWindowBuffer() = default;

        virtual ~ICountWindowBuffer() = default;

        ICountWindowBuffer(ICountWindowBuffer const&) = delete;
        ICountWindowBuffer& operator=(ICountWindowBuffer const&) = delete;
        ICountWindowBuffer(ICountWindowBuffer&&) = delete;
        ICountWindowBuffer& operator=(ICountWindowBuffer&&) = delete;

        virtual void begin() = 0;
        virtual void push(std::size_t fieldIndex, const void* rawData) = 0;
        virtual void end(core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs) = 0;

        [[nodiscard]] virtual bool ready() const = 0;

        [[nodiscard]] virtual std::size_t getRowCount() const = 0;
        [[nodiscard]] virtual const std::byte* getData(std::size_t rowIndex, std::size_t fieldIndex) const = 0;
        [[nodiscard]] virtual core::shared::types::Timestamp getRowStart(std::size_t rowIndex) const = 0;
        [[nodiscard]] virtual core::shared::types::Timestamp getWindowEnd() const = 0;

        virtual void advanceWindow() = 0;

        virtual void close() = 0;

        [[nodiscard]] virtual std::size_t getEmitCount() const = 0;

        [[nodiscard]] virtual core::shared::types::Timestamp getMinTs() const = 0;
    };

    class CountWindowBuffer final : public ICountWindowBuffer {
    public:
        CountWindowBuffer(const details::CountWindowOptions& config,
                          core::logical::port::Port mDataSchema);

        ~CountWindowBuffer() override;

        CountWindowBuffer(CountWindowBuffer const&) = delete;
        CountWindowBuffer& operator=(CountWindowBuffer const&) = delete;
        CountWindowBuffer(CountWindowBuffer&&) = delete;
        CountWindowBuffer& operator=(CountWindowBuffer&&) = delete;


        void begin() override;
        void push(std::size_t fieldIndex, const void* rawData) override;
        void end(core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs) override;

        [[nodiscard]] bool ready() const override;

        [[nodiscard]] std::size_t getRowCount() const override;
        [[nodiscard]] const std::byte* getData(std::size_t rowIndex, std::size_t fieldIndex) const override;
        [[nodiscard]] core::shared::types::Timestamp getRowStart(std::size_t rowIndex) const override;
        [[nodiscard]] core::shared::types::Timestamp getWindowEnd() const override;

        void advanceWindow() override;

        void close() override;

        [[nodiscard]] std::size_t getEmitCount() const override;

        [[nodiscard]] core::shared::types::Timestamp getMinTs() const override;
    private:
        struct Row {
            std::vector<std::byte> data;
            core::shared::types::Timestamp startTs;
            core::shared::types::Timestamp endTs;
        };

        details::CountWindowOptions mConfig;
        storage::storage::RowLayout mRowLayout;
        core::logical::port::Port mSchema;

        std::deque<Row> mRowBuffer;
        std::optional<core::shared::types::Timestamp> mFirstElementStart;
        Row mPendingRow;

        bool mReady = false;
        std::size_t mReadyRowCount = 0;
        std::size_t mReadyEmitCount = 0;
        core::shared::types::Timestamp mReadyEnd = 0;
        bool mFlushed = false;
    };
}

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_RUNTIME_COUNT_WINDOW_BUFFER_HPP