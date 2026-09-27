#include <basic/operators/pipe/join/physical/details/join_side.h>

#include "core/physical/time/time_point.h"

namespace sierra::odyplan::v3::basic::operators::pipe::join::physical::details {
    namespace {
        constexpr inline bool _before(const std::int64_t lft, const std::int64_t rht) {
            if (lft == core::physical::time::INFINITY_SENTINEL) {
                return false;
            }

            if (rht == core::physical::time::INFINITY_SENTINEL) {
                return true;
            }

            return lft < rht;
        }
    }

    JoinSide::JoinSide(core::logical::port::Port schema,
                       const core::runtime::types::RuntimeTypeRegistry *runtimeTypeRegistry,
                       const bool outOfOrder)
        : mOutOfOrder(outOfOrder),
          mLayout(std::move(schema), *runtimeTypeRegistry),
          mRuntimeTypeRegistry(runtimeTypeRegistry),
          mMinEndTs(0),
          mMinStartTs(0) {
    }

    void JoinSide::begin() {
        mLayout.begin();
    }

    void JoinSide::push(const std::uint64_t index, const void* data) {
        mLayout.push(index, data);
    }

    void JoinSide::compactIfNeeded() {
        if (mRemovedCount == 0) {
            return;
        }

        if (mBufferedRows.size() < 10000) {
            return;
        }

        if (static_cast<double>(mRemovedCount) / mBufferedRows.size() < 0.50) {
            return;
        }

        compact();
    }

    void JoinSide::compact() {
        if (mRemovedCount == 0) {
            return;
        }

        std::deque<RowBuffer> newRows;

        mMinEndTs = std::numeric_limits<core::shared::types::Timestamp>::max();
        mMinStartTs = std::numeric_limits<core::shared::types::Timestamp>::max();

        for (auto& row : mBufferedRows) {
            if (!row.isRemoved) {
                if (row.endTs < mMinEndTs) {
                    mMinEndTs = row.endTs;
                }

                if (row.startTs < mMinStartTs) {
                    mMinStartTs = row.startTs;
                }

                newRows.push_back(std::move(row));
            } else {
                mLayout.releaseRow(row.values);
            }
        }

        mBufferedRows = std::move(newRows);
        mBasisIndex = 0;
        mRemovedCount = 0;

        rebuildKeyIndex();
    }

    void JoinSide::rebuildKeyIndex() {
        mKeyIndex.clear();

        if (mKeyFields.empty()) {
            return;
        }

        for (std::size_t i = 0; i < mBufferedRows.size(); ++i) {
            if (mBufferedRows[i].isRemoved) {
                continue;
            }

            const auto* keyPtr = mBufferedRows[i].values.data();
            const auto key = computeRowKeyV(keyPtr);
            const auto absIndex = mBasisIndex + i;
            mKeyIndex.emplace(key, absIndex);
        }
    }

    void JoinSide::removeFromKeyIndex(const RowBuffer& row, std::size_t pos) {
        if (mKeyFields.empty()) {
            return;
        }

        const auto* keyPtr = row.values.data();
        const auto key = computeRowKeyV(keyPtr);
        const auto absIndex = mBasisIndex + pos;

        auto [v, g] = mKeyIndex.equal_range(key);

        for (auto iter = v; iter != g; ++iter) {
            if (iter->second == absIndex) {
                mKeyIndex.erase(iter);
                break;
            }
        }
    }

    void JoinSide::evict(const core::shared::types::Timestamp threshold, const bool isFinal) {
        if (!mOutOfOrder) {
            if (!isFinal && !_before(mMinEndTs, threshold)) {
                return;
            }

            std::size_t evictedCount = 0;
            for (std::size_t i = 0; i < mBufferedRows.size(); ++i) {
                auto& row = mBufferedRows[i];

                if (row.isRemoved) {
                    continue;
                }

                if (isFinal || _before(row.endTs, threshold)) {
                    removeFromKeyIndex(row, i);

                    row.isRemoved = true;
                    evictedCount++;
                }
            }

            if (evictedCount > 0) {
                compactIfNeeded();
            }
        } else { //InOrder
            while (!mBufferedRows.empty()) {
                auto& front = mBufferedRows.front();

                if (front.isRemoved) {
                    mBufferedRows.pop_front();
                    mLayout.releaseRow(front.values);
                    continue;
                }

                if (!isFinal && !_before(front.endTs, threshold)) {
                    break;
                }

                removeFromKeyIndex(front, 0);

                mLayout.releaseRow(front.values);
                mBufferedRows.pop_front();
                ++mBasisIndex;
            }
        }
    }

    std::int64_t JoinSide::getStartTibreaker(const std::uint64_t rowIndex) const {
        return mBufferedRows.at(rowIndex - mBasisIndex).startTb;
    }

    std::int64_t JoinSide::getEndTibreaker(const std::uint64_t rowIndex) const {
        return mBufferedRows.at(rowIndex - mBasisIndex).endTb;
    }

    void JoinSide::end(const core::shared::types::Timestamp startTs, const core::shared::types::Timestamp endTs, const std::int64_t startTb, const std::int64_t endTb) {
        const auto absoluteIndex = mBasisIndex + mBufferedRows.size();

        mBufferedRows.push_back(
            RowBuffer{
                .values = mLayout.pop(),
                .startTs = startTs,
                .endTs = endTs,
                .startTb = startTb,
                .endTb = endTb,
                .isRemoved = false
            });

        if (mBufferedRows.size() == 1 || startTs < mMinStartTs) {
            mMinStartTs = startTs;
        }

        if (mBufferedRows.size() == 1 || endTs < mMinEndTs) {
            mMinEndTs = endTs;
        }

        if (!mKeyFields.empty()) {
            const auto* keyPtr =
                mBufferedRows.back().values.data();
            mKeyIndex.emplace(computeRowKeyV(keyPtr), absoluteIndex);
        }

        mLastAbsoluteIndex = absoluteIndex;
    }

    void JoinSide::release() {
        for (auto& row : mBufferedRows) {
            mLayout.releaseRow(row.values);
        }
        mBufferedRows.clear();
        mKeyIndex.clear();
        mRemovedCount = 0;
    }

    std::uint64_t JoinSide::getSize() const {
        return mBufferedRows.size();
    }

    const std::byte* JoinSide::getFieldPtr(const std::uint64_t rowIndex,
                                           const std::uint64_t fieldIndex) const {
        return mBufferedRows.at(rowIndex - mBasisIndex).values.data() +
               mLayout.offsetOf(fieldIndex);
    }

    bool JoinSide::overlaps(const core::shared::types::Timestamp otherStartTs, const core::shared::types::Timestamp otherEndTs, const std::uint64_t rowIndex) const {
        const auto& row = mBufferedRows.at(rowIndex - mBasisIndex);

        if (row.isRemoved) {
            return false;
        }

        return _before(row.startTs, otherEndTs) && _before(otherStartTs, row.endTs);
    }

    core::shared::types::Timestamp JoinSide::getStartTimestamp(const std::uint64_t rowIndex) const {
        return mBufferedRows.at(rowIndex - mBasisIndex).startTs;
    }

    core::shared::types::Timestamp JoinSide::getEndTimestamp(const std::uint64_t rowIndex) const {
        return mBufferedRows.at(rowIndex - mBasisIndex).endTs;
    }

    void JoinSide::enableKeyIdx(const std::vector<std::uint64_t>& fieldIndices,
        const std::vector<core::shared::types::ValueTypeId>& keyTypeIds) {
        mKeyFields.clear();
        mKeyFields.reserve(fieldIndices.size());

        for (std::size_t i = 0; i < fieldIndices.size(); ++i) {
            mKeyFields.push_back(KeyFieldDetails{
                .fieldIndex=fieldIndices.at(i),
                .mHashFunction=mRuntimeTypeRegistry->getHashFnPtr(keyTypeIds.at(i)),
                .mEqualFunction=mRuntimeTypeRegistry->getEqualFnPtr(keyTypeIds.at(i))
            });
        }
    }

    std::vector<std::uint64_t> JoinSide::getKeyFieldIndices() const {
        std::vector<std::uint64_t> result;
        result.reserve(mKeyFields.size());
        for (const auto& field : mKeyFields) {
            result.push_back(field.fieldIndex);
        }
        return result;
    }

    bool JoinSide::hasKeyIndex() const {
        return !mKeyFields.empty();
    }

    std::uint64_t JoinSide::getLastAbsoluteIndex() const {
        return mLastAbsoluteIndex;
    }

    std::vector<std::uint64_t> JoinSide::computeRowKeyV(const std::byte* row) const {
        std::vector<std::uint64_t> result;
        result.reserve(mKeyFields.size());

        for (const auto& field : mKeyFields) {
            const auto* fieldPtr = row + mLayout.offsetOf(field.fieldIndex);
            result.push_back(field.mHashFunction(fieldPtr));
        }

        return result;
    }

    std::vector<std::uint64_t> JoinSide::hashLookup(const std::vector<const std::byte*>& query) {
        std::vector<std::uint64_t> result;

        std::vector<std::uint64_t> qKey;
        qKey.reserve(mKeyFields.size());
        for (std::size_t i = 0; i < mKeyFields.size(); ++i) {
            qKey.push_back(mKeyFields.at(i).mHashFunction(query.at(i)));
        }

        const auto [z, h] = mKeyIndex.equal_range(qKey);

        for (auto iter = z; iter != h; ++iter) {
            const auto candidateIndex = iter->second;
            const auto locIndex = candidateIndex - mBasisIndex;

            if (locIndex >= mBufferedRows.size()) {
                continue;
            }

            if (mBufferedRows[locIndex].isRemoved) {
                continue;
            }

            const auto* candidateRowPtr = getRowPtr(candidateIndex);

            bool allMatch = true;
            for (std::size_t i = 0; i < mKeyFields.size(); ++i) {
                const auto* candidateFieldPtr = candidateRowPtr + mLayout.offsetOf(mKeyFields.at(i).fieldIndex);
                if (!mKeyFields.at(i).mEqualFunction(candidateFieldPtr,query.at(i))) {
                    allMatch = false;
                    break;
                }
            }

            if (allMatch) {
                result.push_back(candidateIndex);
            }
        }
        return result;
    }

    std::uint64_t JoinSide::getBaseIndex() const {
        return mBasisIndex;
    }

    const std::byte* JoinSide::getRowPtr(const std::uint64_t rowIndex) const {
        return mBufferedRows.at(rowIndex - mBasisIndex).values.data();
    }

    std::uint64_t JoinSide::getRowByteSize() const {
        return mLayout.getTotalByteSize();
    }

    std::uint64_t JoinSide::getFieldCount() const {
        return mLayout.getFieldCount();
    }

    core::shared::types::ValueTypeId JoinSide::getFieldTypeId(const std::uint64_t index) const {
        return mLayout.getFieldTypeId(index);
    }

    std::uint64_t JoinSide::getFieldOffset(const std::uint64_t index) const {
        return mLayout.offsetOf(index);
    }

    void JoinSide::setPending(const core::shared::types::Timestamp startTs, const core::shared::types::Timestamp endTs, const std::int64_t startTb, const std::int64_t endTb) {
        mPendingStartTs = startTs;
        mPendingStartTb = startTb;
        mPendingEndTs = endTs;
        mPendingEndTb = endTb;
    }

    void JoinSide::removeV(const std::vector<std::size_t>& absIndices) {
        if (absIndices.empty()) {
            return;
        }

        std::vector<std::size_t> sorted = absIndices;
        std::ranges::sort(sorted, std::greater<>());

        for (const auto absIndex : sorted) {
            const std::size_t locIndex = absIndex - mBasisIndex;
            if (locIndex >= mBufferedRows.size()) {
                continue;
            }

            auto& row = mBufferedRows[locIndex];

            if (row.isRemoved) {
                continue;
            }

            removeFromKeyIndex(row, locIndex);

            //mLayout.releaseRow(mBufferedRows[locIndex].values);
            row.isRemoved = true;
            mRemovedCount++;
        }

        compactIfNeeded();
    }

    const std::byte* JoinSide::getLayoutFieldPtr(std::uint64_t fieldIndex) const {
        return mLayout.getCurrentRowPtr() + mLayout.offsetOf(fieldIndex);
    }

    std::uint64_t JoinSide::getCurrentLayoutRowSize() const {
        return mLayout.getCurrentRowSize();
    }

    const std::byte* JoinSide::getLayoutRowPtr() const {
        return mLayout.getCurrentRowPtr();
    }

    core::shared::types::Timestamp JoinSide::getLayoutStartTs() const {
        return mPendingStartTs;
    }

    core::shared::types::Timestamp JoinSide::getLayoutEndTs() const {
        return mPendingEndTs;
    }

    std::int64_t JoinSide::getLayoutStartTb() const {
        return mPendingStartTb;
    }

    std::int64_t JoinSide::getLayoutEndTb() const {
        return mPendingEndTb;
    }

    const core::logical::port::Port& JoinSide::getSchema() const {
        return mLayout.getSchema();
    }

    core::shared::types::Timestamp JoinSide::getMinStartTimestamp() const {
        return mMinStartTs;
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::physical::details