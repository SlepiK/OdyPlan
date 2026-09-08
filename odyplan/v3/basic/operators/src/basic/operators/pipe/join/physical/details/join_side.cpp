#include <basic/operators/pipe/join/physical/details/join_side.h>

namespace sierra::odyplan::v3::basic::operators::pipe::join::physical::details {
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

    void JoinSide::evict(const Timestamp threshold) {
        while (!mBufferedRows.empty() &&
               mBufferedRows.front().endTs <= threshold) {
            if (!mKeyFields.empty()) {
                const auto* keyPtr = mBufferedRows.front().values.data();
                const auto key = computeRowKeyV(keyPtr);
                auto [v, g] = mKeyIndex.equal_range(key);
                for (auto iter = v; iter != g; ++iter) {
                    if (iter->second == mBasisIndex) {
                        mKeyIndex.erase(iter);
                        break;
                    }
                }
            }
            mLayout.releaseRow(mBufferedRows.front().values);
            mBufferedRows.pop_front();
            ++mBasisIndex;
        }
    }

    void JoinSide::end(const Timestamp startTs, const Timestamp endTs) {
        const auto absoluteIndex = mBasisIndex + mBufferedRows.size();

        mBufferedRows.push_back(RowBuffer{.values = mLayout.pop(), .startTs = startTs, .endTs = endTs});

        if (!mKeyFields.empty()) {
            const auto* keyPtr =
                mBufferedRows.back().values.data();
            mKeyIndex.emplace(computeRowKeyV(keyPtr), absoluteIndex);
        }

        mLastAbsoluteIndex = absoluteIndex;
    }

    void JoinSide::release() {
        for (auto& [values, timestampMs] : mBufferedRows) {
            mLayout.releaseRow(values);
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
        return row.startTs < otherEndTs && otherStartTs < row.endTs;
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

}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::physical::details