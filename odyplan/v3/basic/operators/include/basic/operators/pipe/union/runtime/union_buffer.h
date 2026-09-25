#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_RUNTIME_UNION_BUFFER_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_RUNTIME_UNION_BUFFER_HPP

#include <atomic>
#include <queue>
#include <basic/window/window_row_layout.h>
#include <core/physical/time/time_point.h>
#include <mutex>
#include <shared_mutex>
#include <core/shared/identifiers/port_id.h>

namespace sierra::odyplan::v3::basic::operators::pipe::union_::runtime {// NOLINT(readability-identifier-naming)
    using Timestamp = std::int64_t;

    class UnionBuffer {
    public:
        struct BatchResult {
            std::uint64_t count;
            std::uint64_t *indices;
        };

        explicit UnionBuffer(bool outOfOrder,
                             core::logical::port::Port schema,
                             const core::runtime::types::RuntimeTypeRegistry *runtimeTypeRegistry);

        ~UnionBuffer();

        UnionBuffer(const UnionBuffer &) = delete;
        UnionBuffer &operator=(const UnionBuffer &) = delete;
        UnionBuffer(UnionBuffer &&) = delete;
        UnionBuffer &operator=(UnionBuffer &&) = delete;

        void lock();

        void unlock();

        void lockShared();

        void unlockShared();

        void begin();

        void push(std::uint64_t index, const void *data);

        void end(core::shared::identifiers::PortId inputPort,
                 Timestamp startTs, Timestamp endTs,
                 std::int64_t startTb, std::int64_t endTb);

        [[nodiscard]] std::uint64_t getCount() const;

        [[nodiscard]] bool isEmpty() const;

        [[nodiscard]] Timestamp getMinStartTimestamp();

        [[nodiscard]] const std::byte *getData(std::uint64_t index) const;

        [[nodiscard]] const std::byte *getFieldPtr(std::uint64_t rowIndex, std::uint64_t fieldIndex) const;

        [[nodiscard]] Timestamp getStartTimestamp(std::uint64_t index) const;

        [[nodiscard]] Timestamp getEndTimestamp(std::uint64_t index) const;

        [[nodiscard]] std::int64_t getStartTibreaker(std::uint64_t index) const;

        [[nodiscard]] std::int64_t getEndTibreaker(std::uint64_t index) const;

        [[nodiscard]] core::shared::identifiers::PortId getInputPort(std::uint64_t index) const;

        void removeV(const void *indicesPtr, std::int64_t count);

        void clear();

        [[nodiscard]] std::uint64_t getFieldCount() const;

        [[nodiscard]] core::shared::types::ValueTypeId getFieldTypeId(std::uint64_t index) const;

        [[nodiscard]] std::uint64_t getFieldOffset(std::uint64_t index) const;

        [[nodiscard]] std::uint64_t getRowByteSize() const;

        [[nodiscard]] std::optional<std::uint64_t> getMinStartTimestampIndex(Timestamp watermark);

        [[nodiscard]] BatchResult getReadyIndices(Timestamp watermark, std::size_t maxCount = 32);

    private:
        struct RowMeta {
            std::vector<std::byte> values;
            Timestamp startTs = 0;
            Timestamp endTs = core::physical::time::INFINITY_SENTINEL;
            std::int64_t startTb = 0;
            std::int64_t endTb = 0;
            core::shared::identifiers::PortId inputPort = 0;
            bool isRemoved = false;
        };

        static constexpr std::size_t MAX_BATCH_SIZE = 64;

        std::shared_mutex mMutex;
        const bool mOutOfOrder;
        const core::runtime::types::RuntimeTypeRegistry *mRuntimeTypeRegistry;

        window::WindowRowLayout mLayout;
        std::vector<RowMeta> mRowMetas;

        std::vector<std::uint64_t> mRowIndex;

        Timestamp mMaxSortedWatermark = 0;
        Timestamp mLastEmittedWatermark = 0;

        std::size_t mActiveRowCount = 0;
        std::uint64_t mRemovedCount = 0;
        std::array<std::uint64_t, MAX_BATCH_SIZE> mBatchBuffer;

        void drainEmittedEntries(Timestamp watermark);
        void sorted(Timestamp watermark);
        void compactIfNeeded();
        void compact();

        static inline bool compareRows(const Timestamp tsA, const std::int64_t tbA, const Timestamp tsB, const std::int64_t tbB) {
            if (tsA != tsB) {
                return tsA < tsB;
            }
            return tbA < tbB;
        }
    };
}
#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_RUNTIME_UNION_BUFFER_HPP