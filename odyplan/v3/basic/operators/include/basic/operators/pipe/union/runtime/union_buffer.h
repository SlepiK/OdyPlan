#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_RUNTIME_UNION_BUFFER_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_RUNTIME_UNION_BUFFER_HPP

#include <queue>
#include <basic/window/window_row_layout.h>
#include <core/physical/time/time_point.h>
#include <mutex>
#include <core/shared/identifiers/port_id.h>

namespace sierra::odyplan::v3::basic::operators::pipe::union_::runtime {// NOLINT(readability-identifier-naming)
    using Timestamp = std::int64_t;

    class UnionBuffer {
    public:
        struct RetainPointer {
            core::shared::types::ValueTypeId typeId;
            void* ptr;
        };

        UnionBuffer(bool outOfOrder, core::logical::port::Port schema, const core::runtime::types::RuntimeTypeRegistry* runtimeTypeRegistry);
        ~UnionBuffer();

        UnionBuffer(const UnionBuffer&) = delete;
        UnionBuffer& operator=(const UnionBuffer&) = delete;
        UnionBuffer(UnionBuffer&&) = delete;
        UnionBuffer& operator=(UnionBuffer&&) = delete;

        void lock();
        void unlock();

        void begin();
        void push(std::uint64_t index, const void* data);
        void end(core::shared::identifiers::PortId inputPort, Timestamp startTs, Timestamp endTs, std::int64_t startTb, std::int64_t endTb);

        [[nodiscard]] std::uint64_t getCount() const;
        [[nodiscard]] bool isEmpty() const;
        [[nodiscard]] Timestamp getMinStartTimestamp();
        [[nodiscard]] const std::byte* getData(std::uint64_t index) const;
        [[nodiscard]] const std::byte* getFieldPtr(std::uint64_t rowIndex,
                                                         std::uint64_t fieldIndex) const;
        [[nodiscard]] Timestamp getStartTimestamp(std::uint64_t index) const;
        [[nodiscard]] Timestamp getEndTimestamp(std::uint64_t index) const;
        [[nodiscard]] std::int64_t getStartTibreaker(std::uint64_t index) const;
        [[nodiscard]] std::int64_t getEndTibreaker(std::uint64_t index) const;
        [[nodiscard]] core::shared::identifiers::PortId getInputPort(std::uint64_t index) const;

        void removeV(const std::vector<std::uint64_t>& indices);
        void clear();

        [[nodiscard]] std::uint64_t getFieldCount() const;
        [[nodiscard]] core::shared::types::ValueTypeId getFieldTypeId(std::uint64_t index) const;
        [[nodiscard]] std::uint64_t getFieldOffset(std::uint64_t index) const;
        [[nodiscard]] std::uint64_t getRowByteSize() const;
        [[nodiscard]] std::optional<std::uint64_t> getMinStartTimestampIndex(Timestamp watermark);
    private:
        struct RowBuffer {
            std::vector<std::byte> values;

            core::shared::identifiers::PortId inputPort;

            Timestamp startTs = 0;
            Timestamp endTs = core::physical::time::INFINITY_SENTINEL;
            std::int64_t startTb = 0;
            std::int64_t endTb = 0;

            bool isRemoved = false;
            std::uint64_t rowIndex;
        };

        struct CompareByStartTs {
            bool operator()(const RowBuffer& a, const RowBuffer& b) const {
                if (a.startTs != b.startTs) {
                    return a.startTs > b.startTs;
                }
                return a.startTb > b.startTb;
            }
        };

        std::mutex mMutex;
        bool mOutOfOrder;
        const core::runtime::types::RuntimeTypeRegistry* mRuntimeTypeRegistry;
        std::size_t mActiveRowCount;

        std::priority_queue<RowBuffer, std::vector<RowBuffer>,CompareByStartTs> mRowIndex;
        std::deque<RowBuffer> mRows;
        window::WindowRowLayout mRowLayout;
    };
}
#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_RUNTIME_UNION_BUFFER_HPP