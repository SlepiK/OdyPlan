#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_DETAILS_JOIN_SIDE_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_DETAILS_JOIN_SIDE_HPP
#include <basic/window/window_row_layout.h>
#include <core/logical/port/port.h>
#include <deque>
#include <map>

#include "core/physical/time/time_point.h"

namespace sierra::odyplan::v3::basic::operators::pipe::join::physical::details {
    using Timestamp = std::int64_t;

    class JoinSide {
    public:
        JoinSide(core::logical::port::Port schema,
                 const core::runtime::types::RuntimeTypeRegistry* runtimeTypeRegistry);

        void begin();

        void push(std::uint64_t index, const void* data);

        void evict(Timestamp threshold, bool isFinal);

        void end(Timestamp startTs, Timestamp endTs, std::int64_t startTb, std::int64_t endTb);

        void release();

        [[nodiscard]] std::uint64_t getSize() const;

        [[nodiscard]] const std::byte* getFieldPtr(std::uint64_t rowIndex,
                                                   std::uint64_t fieldIndex) const;

        [[nodiscard]] bool overlaps(Timestamp otherStartTs, Timestamp otherEndTs, std::uint64_t rowIndex) const;

        [[nodiscard]] Timestamp getStartTimestamp(std::uint64_t rowIndex) const;

        [[nodiscard]] Timestamp getEndTimestamp(std::uint64_t rowIndex) const;

        [[nodiscard]] std::int64_t getStartTibreaker(std::uint64_t rowIndex) const;

        [[nodiscard]] std::int64_t getEndTibreaker(std::uint64_t rowIndex) const;

        void enableKeyIdx(const std::vector<std::uint64_t>& fieldIndices, const std::vector<core::shared::types::ValueTypeId>& keyTypeIds);

        [[nodiscard]] std::vector<std::uint64_t> getKeyFieldIndices() const;

        [[nodiscard]] bool hasKeyIndex() const;

        [[nodiscard]] std::uint64_t getLastAbsoluteIndex() const;

        [[nodiscard]] std::vector<std::uint64_t> hashLookup(const std::vector<const std::byte*>& query);

        [[nodiscard]] std::uint64_t getBaseIndex() const;

        [[nodiscard]] const std::byte* getRowPtr(std::uint64_t rowIndex) const;

        [[nodiscard]] std::uint64_t getRowByteSize() const;

        [[nodiscard]] std::uint64_t getFieldCount() const;

        [[nodiscard]] core::shared::types::ValueTypeId getFieldTypeId(std::uint64_t index) const;

        [[nodiscard]] std::uint64_t getFieldOffset(std::uint64_t index) const;

        void removeV(const std::vector<std::size_t>& absIndices);

        [[nodiscard]] const std::byte* getLayoutFieldPtr(std::uint64_t fieldIndex) const;

        [[nodiscard]] const std::byte* getLayoutRowPtr() const;

        [[nodiscard]] std::uint64_t getCurrentLayoutRowSize() const;

        [[nodiscard]] Timestamp getLayoutStartTs() const;

        [[nodiscard]] Timestamp getLayoutEndTs() const;

        [[nodiscard]] std::int64_t getLayoutStartTb() const;

        [[nodiscard]] std::int64_t getLayoutEndTb() const;

        void setPending(Timestamp startTs, Timestamp endTs, std::int64_t startTb, std::int64_t endTb);

    private:
        struct RowBuffer {
            std::vector<std::byte> values;
            Timestamp startTs = 0;
            Timestamp endTs = core::physical::time::INFINITY_SENTINEL;
            std::int64_t startTb = 0;
            std::int64_t endTb = 0;
        };

        struct KeyFieldDetails {
            std::uint64_t fieldIndex;
            core::runtime::types::RuntimeTypeRegistry::HashFunctionPtr mHashFunction;
            core::runtime::types::RuntimeTypeRegistry::EqualFunctionPtr mEqualFunction;
        };

        Timestamp mPendingStartTs;
        Timestamp mPendingEndTs;
        std::int64_t mPendingStartTb;
        std::int64_t mPendingEndTb;

        window::WindowRowLayout mLayout;
        std::deque<RowBuffer> mBufferedRows;
        // Hash-Behavior for == predicate only

        std::multimap<std::vector<std::uint64_t>, std::uint64_t> mKeyIndex;
        std::vector<KeyFieldDetails> mKeyFields;
        const core::runtime::types::RuntimeTypeRegistry* mRuntimeTypeRegistry;

        std::uint64_t mBasisIndex = 0;
        std::uint64_t mLastAbsoluteIndex = 0;

        std::vector<std::uint64_t> computeRowKeyV(const std::byte* row) const;
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::physical::details

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_DETAILS_JOIN_SIDE_HPP