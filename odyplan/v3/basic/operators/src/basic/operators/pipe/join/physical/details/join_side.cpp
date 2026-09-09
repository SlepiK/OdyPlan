#include <basic/operators/pipe/join/physical/details/join_side.h>

#include "core/physical/time/time_point.h"

namespace sierra::odyplan::v3::basic::operators::pipe::join::physical::details {
    namespace {
        inline bool _before(const std::int64_t lft, const std::int64_t rht) {
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
                       const core::runtime::types::RuntimeTypeRegistry* runtimeTypeRegistry)
        : mLayout(std::move(schema), *runtimeTypeRegistry),
          mRuntimeTypeRegistry(runtimeTypeRegistry) {
    }

    void JoinSide::begin() {
        mLayout.begin();
    }

    void JoinSide::push(const std::uint64_t index, const void* data) {
        mLayout.push(index, data);
    }

    void JoinSide::evict(const Timestamp threshold, const bool isFinal) {
        while (!mBufferedRows.empty()) {
            const auto& front = mBufferedRows.front();
            if (!isFinal && !_before(front.endTs, threshold)) {
                break;
            }
            if (!mKeyFields.empty()) {
                const auto* keyPtr = front.values.data();
                const auto key = computeRowKeyV(keyPtr);
                auto [v, g] = mKeyIndex.equal_range(key);
                for (auto iter = v; iter != g; ++iter) {
                    if (iter->second == mBasisIndex) {
                        mKeyIndex.erase(iter);
                        break;
                    }
                }
            }
            mLayout.releaseRow(front.values);
            mBufferedRows.pop_front();
            ++mBasisIndex;
        }
    }

    std::int64_t JoinSide::getStartTibreaker(const std::uint64_t rowIndex) const {
        return mBufferedRows.at(rowIndex - mBasisIndex).startTb;
    }

    std::int64_t JoinSide::getEndTibreaker(const std::uint64_t rowIndex) const {
        return mBufferedRows.at(rowIndex - mBasisIndex).endTb;
    }

    void JoinSide::end(const Timestamp startTs, const Timestamp endTs, const std::int64_t startTb, const std::int64_t endTb) {
        const auto absoluteIndex = mBasisIndex + mBufferedRows.size();

        mBufferedRows.push_back(
            RowBuffer{
                .values = mLayout.pop(),
                .startTs = startTs,
                .endTs = endTs,
                .startTb = startTb,
                .endTb = endTb
            });

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
    }

    std::uint64_t JoinSide::getSize() const {
        return mBufferedRows.size();
    }

    const std::byte* JoinSide::getFieldPtr(const std::uint64_t rowIndex,
                                           const std::uint64_t fieldIndex) const {
        return mBufferedRows.at(rowIndex - mBasisIndex).values.data() +
               mLayout.offsetOf(fieldIndex);
    }

    bool JoinSide::overlaps(const Timestamp otherStartTs, const Timestamp otherEndTs, const std::uint64_t rowIndex) const {
        const auto& row = mBufferedRows.at(rowIndex - mBasisIndex);
        return _before(row.startTs, otherEndTs) && _before(otherStartTs, row.endTs);
    }

    Timestamp JoinSide::getStartTimestamp(const std::uint64_t rowIndex) const {
        return mBufferedRows.at(rowIndex - mBasisIndex).startTs;
    }

    Timestamp JoinSide::getEndTimestamp(const std::uint64_t rowIndex) const {
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

    void JoinSide::setPending(const Timestamp startTs, const Timestamp endTs, const std::int64_t startTb, const std::int64_t endTb) {
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

            if (!mKeyFields.empty()) {
                const auto* keyPtr = mBufferedRows.at(locIndex).values.data();
                const auto key = computeRowKeyV(keyPtr);
                auto [vi, g] = mKeyIndex.equal_range(key);
                for (auto iter = vi; iter != g; ++iter) {
                    if (iter->second == absIndex) {
                        mKeyIndex.erase(iter);
                        break;
                    }
                }
            }

            mLayout.releaseRow(mBufferedRows[locIndex].values);
            mBufferedRows.erase(mBufferedRows.begin() + static_cast<std::ptrdiff_t>(locIndex));
        }
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

    Timestamp JoinSide::getLayoutStartTs() const {
        return mPendingStartTs;
    }

    Timestamp JoinSide::getLayoutEndTs() const {
        return mPendingEndTs;
    }

    std::int64_t JoinSide::getLayoutStartTb() const {
        return mPendingStartTb;
    }

    std::int64_t JoinSide::getLayoutEndTb() const {
        return mPendingEndTb;
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::physical::details