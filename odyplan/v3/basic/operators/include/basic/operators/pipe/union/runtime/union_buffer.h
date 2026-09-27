#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_RUNTIME_UNION_BUFFER_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_RUNTIME_UNION_BUFFER_HPP

#include <atomic>
#include <queue>
#include <basic/window/window_row_layout.h>
#include <core/physical/time/time_point.h>
#include <mutex>
#include <shared_mutex>
#include <core/shared/identifiers/port_id.h>
#include <core/shared/types/global_types.h>

namespace sierra::odyplan::v3::basic::operators::pipe::union_::runtime {// NOLINT(readability-identifier-naming)
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
                 core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs,
                 core::shared::types::TiBreak startTb, core::shared::types::TiBreak endTb);

        [[nodiscard]] std::uint64_t getCount() const;

        [[nodiscard]] bool isEmpty() const;

        [[nodiscard]] core::shared::types::Timestamp getMinStartTimestamp();

        [[nodiscard]] const std::byte *getData(std::uint64_t index) const;

        [[nodiscard]] const std::byte *getFieldPtr(std::uint64_t rowIndex, std::uint64_t fieldIndex) const;

        [[nodiscard]] core::shared::types::Timestamp getStartTimestamp(std::uint64_t index) const;

        [[nodiscard]] core::shared::types::Timestamp getEndTimestamp(std::uint64_t index) const;

        [[nodiscard]] core::shared::types::TiBreak getStartTibreaker(std::uint64_t index) const;

        [[nodiscard]] core::shared::types::TiBreak getEndTibreaker(std::uint64_t index) const;

        [[nodiscard]] core::shared::identifiers::PortId getInputPort(std::uint64_t index) const;

        void removeV(const void *indicesPtr, std::int64_t count);

        void clear();

        [[nodiscard]] std::uint64_t getFieldCount() const;

        [[nodiscard]] core::shared::types::ValueTypeId getFieldTypeId(std::uint64_t index) const;

        [[nodiscard]] std::uint64_t getFieldOffset(std::uint64_t index) const;

        [[nodiscard]] std::uint64_t getRowByteSize() const;

        [[nodiscard]] std::optional<core::shared::types::Timestamp> getMinStartTimestampIndex(core::shared::types::Timestamp watermark);

        [[nodiscard]] BatchResult getReadyIndices(core::shared::types::Timestamp watermark, std::size_t maxCount = 32);

    private:
        struct RowMeta {
            std::vector<std::byte> values;
            core::shared::types::Timestamp startTs = 0;
            core::shared::types::Timestamp endTs = core::physical::time::INFINITY_SENTINEL;
            core::shared::types::TiBreak startTb = 0;
            core::shared::types::TiBreak endTb = 0;
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

        core::shared::types::Timestamp mLastEmittedWatermark = 0;

        std::size_t mActiveRowCount = 0;
        std::uint64_t mRemovedCount = 0;
        std::array<std::uint64_t, MAX_BATCH_SIZE> mBatchBuffer;

        void drainEmittedEntries(core::shared::types::Timestamp watermark);
        void compactIfNeeded();
        void compact();

        static inline bool compareRows(const core::shared::types::Timestamp tsA, const core::shared::types::TiBreak tbA, const core::shared::types::Timestamp tsB, const core::shared::types::TiBreak tbB) {
            if (tsA != tsB) {
                return tsA < tsB;
            }
            return tbA < tbB;
        }
    };
}
#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_RUNTIME_UNION_BUFFER_HPP