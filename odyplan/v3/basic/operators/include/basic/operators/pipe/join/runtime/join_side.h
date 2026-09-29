#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_RUNTIME_JOIN_SIDE_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_RUNTIME_JOIN_SIDE_HPP
#include <basic/storage/row_layout.h>
#include <core/logical/port/port.h>
#include <deque>
#include <map>

#include <core/physical/time/time_point.h>
#include <core/shared/types/global_types.h>

namespace sierra::odyplan::v3::basic::operators::pipe::join::runtime {
    class JoinSide {
    public:
        JoinSide(const core::runtime::types::RuntimeTypeRegistry& rtRegistry, core::logical::port::Port schema, bool outOfOrder);

        void begin();

        void push(std::size_t index, const void* data);

        void evict(core::shared::types::Timestamp threshold, bool isFinal);

        void end(core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs, core::shared::types::TiBreak startTb, core::shared::types::TiBreak endTb);

        void release();

        [[nodiscard]] std::size_t getSize() const;

        [[nodiscard]] const std::byte* getFieldPtr(std::size_t rowIndex,
                                                   std::size_t fieldIndex) const;

        [[nodiscard]] bool overlaps(core::shared::types::Timestamp otherStartTs, core::shared::types::Timestamp otherEndTs, std::size_t rowIndex) const;

        [[nodiscard]] core::shared::types::Timestamp getStartTimestamp(std::size_t rowIndex) const;

        [[nodiscard]] core::shared::types::Timestamp getEndTimestamp(std::size_t rowIndex) const;

        [[nodiscard]] core::shared::types::TiBreak getStartTibreaker(std::size_t rowIndex) const;

        [[nodiscard]] core::shared::types::TiBreak getEndTibreaker(std::size_t rowIndex) const;

        void enableKeyIdx(const std::vector<std::size_t>& fieldIndices, const std::vector<core::shared::types::ValueTypeId>& keyTypeIds);

        [[nodiscard]] std::vector<std::size_t> getKeyFieldIndices() const;

        [[nodiscard]] bool hasKeyIndex() const;

        [[nodiscard]] std::size_t getLastAbsoluteIndex() const;

        [[nodiscard]] std::vector<std::size_t> hashLookup(const std::vector<const std::byte*>& query);

        [[nodiscard]] std::size_t getBaseIndex() const;

        [[nodiscard]] const std::byte* getRowPtr(std::size_t rowIndex) const;

        [[nodiscard]] std::size_t getRowByteSize() const;

        [[nodiscard]] std::size_t getFieldCount() const;

        [[nodiscard]] core::shared::types::ValueTypeId getFieldTypeId(std::size_t index) const;

        [[nodiscard]] std::size_t getFieldOffset(std::size_t index) const;

        void removeV(const std::vector<std::size_t>& absIndices);

        [[nodiscard]] const std::byte* getLayoutFieldPtr(std::size_t fieldIndex) const;

        [[nodiscard]] const std::byte* getLayoutRowPtr() const;

        [[nodiscard]] std::size_t getCurrentLayoutRowSize() const;

        [[nodiscard]] core::shared::types::Timestamp getLayoutStartTs() const;

        [[nodiscard]] core::shared::types::Timestamp getLayoutEndTs() const;

        [[nodiscard]] core::shared::types::TiBreak getLayoutStartTb() const;

        [[nodiscard]] core::shared::types::TiBreak getLayoutEndTb() const;

        void setPending(core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs, core::shared::types::TiBreak startTb, core::shared::types::TiBreak endTb);

        [[nodiscard]] const core::logical::port::Port& getSchema() const;

        [[nodiscard]] core::shared::types::Timestamp getMinStartTimestamp() const;

    private:
        struct RowBuffer {
            std::vector<std::byte> values;
            core::shared::types::Timestamp startTs = 0;
            core::shared::types::Timestamp endTs = core::shared::types::INFINITY_TIMESTAMP_SENTINEL;
            core::shared::types::TiBreak startTb = 0;
            core::shared::types::TiBreak endTb = 0;
            bool isRemoved = false;
        };

        struct KeyFieldDetails {
            std::size_t fieldIndex;
            core::runtime::types::RuntimeTypeRegistry::HashFunctionPtr mHashFunction;
            core::runtime::types::RuntimeTypeRegistry::EqualFunctionPtr mEqualFunction;
        };

        const core::runtime::types::RuntimeTypeRegistry* mRegistry;

        core::shared::types::Timestamp mPendingStartTs;
        core::shared::types::Timestamp mPendingEndTs;
        core::shared::types::TiBreak mPendingStartTb;
        core::shared::types::TiBreak mPendingEndTb;

        bool mOutOfOrder;
        std::size_t mRemovedCount = 0;
        core::shared::types::Timestamp mMinStartTs = 0;
        core::shared::types::Timestamp mMinEndTs = 0;

        storage::RowLayout mLayout;
        std::deque<RowBuffer> mBufferedRows;
        // Hash-Behavior for == predicate only

        std::multimap<std::vector<std::size_t>, std::size_t> mKeyIndex;
        std::vector<KeyFieldDetails> mKeyFields;

        std::size_t mBasisIndex = 0;
        std::size_t mLastAbsoluteIndex = 0;

        std::vector<std::size_t> computeRowKeyV(const std::byte* row) const;

        void compactIfNeeded();
        void compact();
        void removeFromKeyIndex(const RowBuffer& row, std::size_t pos);
        void rebuildKeyIndex();
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::physical::details

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_RUNTIME_JOIN_SIDE_HPP