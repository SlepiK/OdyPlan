#include <basic/operators/pipe/union/runtime/union_buffer.h>

#include <algorithm>
#include <iostream>

namespace sierra::odyplan::v3::basic::operators::pipe::union_::runtime {// NOLINT(readability-identifier-naming)
    UnionBuffer::UnionBuffer(bool outOfOrder,
                             core::logical::port::Port schema,
                             const core::runtime::types::RuntimeTypeRegistry *runtimeTypeRegistry)
        : mOutOfOrder(outOfOrder)
          , mRuntimeTypeRegistry(runtimeTypeRegistry)
          , mLayout(std::move(schema), *runtimeTypeRegistry)
          , mMaxSortedWatermark(0)
          , mLastEmittedWatermark(0)
          , mRemovedCount(0) {
        mRowMetas.reserve(2048);
        mRowIndex.reserve(2048);
    }

    UnionBuffer::~UnionBuffer() {
        clear();
    }

    void UnionBuffer::lock() {
        mMutex.lock();
    }

    void UnionBuffer::unlock() {
        compactIfNeeded();
        mMutex.unlock();
    }

    void UnionBuffer::lockShared() {
        mMutex.lock_shared();
    }

    void UnionBuffer::unlockShared() {
        mMutex.unlock_shared();
    }

    void UnionBuffer::begin() {
        mLayout.begin();
    }

    void UnionBuffer::push(const std::uint64_t index, const void* data) {
        mLayout.push(index, data);
    }

    void UnionBuffer::end(const core::shared::identifiers::PortId inputPort,
                      const Timestamp startTs, const Timestamp endTs,
                      const std::int64_t startTb, const std::int64_t endTb) {
        if (!mOutOfOrder) {
            return;
        }

        const std::uint64_t rowIndex = mRowMetas.size();
        std::vector<std::byte> rowValues = mLayout.pop();

        mRowMetas.push_back(RowMeta{
            .values = std::move(rowValues),
            .startTs = startTs,
            .endTs = endTs,
            .startTb = startTb,
            .endTb = endTb,
            .inputPort = inputPort,
            .isRemoved = false
        });

        auto it = std::lower_bound(mRowIndex.begin(), mRowIndex.end(), std::make_pair(startTs, startTb),
                                   [this](const std::uint64_t idx, const std::pair<Timestamp, int64_t>& val) {
                                       return compareRows(mRowMetas[idx].startTs, mRowMetas[idx].startTb, val.first, val.second);
                                   });
        mRowIndex.insert(it, rowIndex);

        mActiveRowCount++;
    }

    std::uint64_t UnionBuffer::getCount() const {
        return mActiveRowCount;
    }

    bool UnionBuffer::isEmpty() const {
        return mActiveRowCount == 0;
    }

    Timestamp UnionBuffer::getMinStartTimestamp() {
        if (mRowIndex.empty()) {
            return core::physical::time::INFINITY_SENTINEL;
        }

        sorted(core::physical::time::INFINITY_SENTINEL);

        for (const std::uint64_t idx : mRowIndex) {
            if (idx < mRowMetas.size() && !mRowMetas[idx].isRemoved) {
                return mRowMetas[idx].startTs;
            }
        }

        return core::physical::time::INFINITY_SENTINEL;
    }

    const std::byte* UnionBuffer::getData(const std::uint64_t index) const {
        if (index >= mRowMetas.size() || mRowMetas[index].isRemoved) {
            return nullptr;
        }
        return mRowMetas[index].values.data();
    }

    const std::byte* UnionBuffer::getFieldPtr(const std::uint64_t rowIndex, const std::uint64_t fieldIndex) const {
        if (rowIndex >= mRowMetas.size() || mRowMetas[rowIndex].isRemoved) {
            return nullptr;
        }

        const std::size_t offset = mLayout.offsetOf(fieldIndex);
        return mRowMetas[rowIndex].values.data() + offset;
    }

    Timestamp UnionBuffer::getStartTimestamp(const std::uint64_t index) const {
        if (index >= mRowMetas.size()) {
            return core::physical::time::INFINITY_SENTINEL;
        }
        return mRowMetas[index].startTs;
    }

    Timestamp UnionBuffer::getEndTimestamp(const std::uint64_t index) const {
        if (index >= mRowMetas.size()) {
            return core::physical::time::INFINITY_SENTINEL;
        }
        return mRowMetas[index].endTs;
    }

    std::int64_t UnionBuffer::getStartTibreaker(const std::uint64_t index) const {
        if (index >= mRowMetas.size()) {
            return core::physical::time::INFINITY_SENTINEL;
        }
        return mRowMetas[index].startTb;
    }

    std::int64_t UnionBuffer::getEndTibreaker(const std::uint64_t index) const {
        if (index >= mRowMetas.size()) {
            return core::physical::time::INFINITY_SENTINEL;
        }
        return mRowMetas[index].endTb;
    }

    core::shared::identifiers::PortId UnionBuffer::getInputPort(const std::uint64_t index) const {
        if (index >= mRowMetas.size()) {
            return 0;
        }
        return mRowMetas[index].inputPort;
    }

    void UnionBuffer::removeV(const void* indicesPtr, const std::int64_t count) {
        const auto* indices = static_cast<const std::uint64_t*>(indicesPtr);
        std::int64_t actuallyRemoved = 0;

        for (std::int64_t i = 0; i < count; ++i) {
            const std::uint64_t index = indices[i];

            if (index < mRowMetas.size() && !mRowMetas[index].isRemoved) {
                mLayout.releaseRow(mRowMetas[index].values);
                mRowMetas[index].isRemoved = true;
                actuallyRemoved++;
                mRemovedCount++;
            }
        }

        if (actuallyRemoved > 0 && mActiveRowCount >= static_cast<std::size_t>(actuallyRemoved)) {
            mActiveRowCount -= actuallyRemoved;
        }

        std::vector<std::uint64_t> newRowIndex;
        newRowIndex.reserve(mRowIndex.size());
        for (std::uint64_t idx : mRowIndex) {
            if (idx >= mRowMetas.size()) {
                continue;
            }

            if (mRowMetas[idx].isRemoved) {
                continue;
            }
            newRowIndex.push_back(idx);
        }
        mRowIndex = std::move(newRowIndex);
    }

    void UnionBuffer::clear() {
        for (auto& meta : mRowMetas) {
            if (!meta.isRemoved) {
                mLayout.releaseRow(meta.values);
            }
        }
        mRowMetas.clear();
        mRowMetas.shrink_to_fit();

        mRowIndex.clear();
        mRowIndex.shrink_to_fit();

        mActiveRowCount = 0;
        mMaxSortedWatermark = 0;
        mLastEmittedWatermark = 0;
        mRemovedCount = 0;
    }

    std::uint64_t UnionBuffer::getFieldCount() const {
        return mLayout.getFieldCount();
    }

    core::shared::types::ValueTypeId UnionBuffer::getFieldTypeId(const std::uint64_t index) const {
        return mLayout.getFieldTypeId(index);
    }

    std::uint64_t UnionBuffer::getFieldOffset(const std::uint64_t index) const {
        return mLayout.offsetOf(index);
    }

    std::uint64_t UnionBuffer::getRowByteSize() const {
        return mLayout.getTotalByteSize();
    }

    std::optional<std::uint64_t> UnionBuffer::getMinStartTimestampIndex(const Timestamp watermark) {
        if (mRowMetas.empty()) {
            return std::nullopt;
        }

        sorted(watermark);

        for (std::uint64_t idx : mRowIndex) {
            if (idx >= mRowMetas.size()) {
                continue;
            }

            const auto& meta = mRowMetas[idx];

            if (meta.isRemoved) {
                continue;
            }

            if (meta.startTs > watermark) {
                return std::nullopt;
            }

            return idx;
        }

        return std::nullopt;
    }

    UnionBuffer::BatchResult UnionBuffer::getReadyIndices(const Timestamp watermark, const std::size_t maxCount) {
        if (mActiveRowCount == 0) {
            return BatchResult{.count = 0, .indices = mBatchBuffer.data()};
        }

        sorted(watermark);

        const auto max = std::min(maxCount, MAX_BATCH_SIZE);
        std::size_t foundCount = 0;

        for (const std::uint64_t idx : mRowIndex) {
            if (foundCount >= max) {
                break;
            }

            if (idx >= mRowMetas.size()) {
                continue;
            }

            const auto& meta = mRowMetas[idx];

            if (meta.isRemoved) {
                continue;
            }

            if (meta.startTs > watermark) {
                break;
            }

            mBatchBuffer[foundCount++] = idx;
        }

        return BatchResult{
            .count = foundCount,
            .indices = mBatchBuffer.data()
        };
    }

    void UnionBuffer::drainEmittedEntries(const Timestamp watermark) {
        if (watermark <= mLastEmittedWatermark) {
            return;
        }

        std::size_t writeIdx = 0;

        for (std::size_t readIdx = 0; readIdx < mRowIndex.size(); ++readIdx) {
            std::uint64_t idx = mRowIndex[readIdx];
            if (idx >= mRowMetas.size()) {
                continue;
            }

            if (const auto& meta = mRowMetas[idx]; !meta.isRemoved && meta.startTs > watermark) {
                mRowIndex[writeIdx++] = idx;
            }
        }

        mRowIndex.resize(writeIdx);
        mLastEmittedWatermark = watermark;
    }

    void UnionBuffer::sorted(const Timestamp watermark) {
        if (watermark <= mMaxSortedWatermark) {
            return;
        }

        if (mRowIndex.empty()) {
            mMaxSortedWatermark = watermark;
            return;
        }

        std::sort(mRowIndex.begin(), mRowIndex.end(),
        [this](std::uint64_t a, std::uint64_t b) {
            const auto& metaA = mRowMetas[a];
            const auto& metaB = mRowMetas[b];

            return compareRows(metaA.startTs, metaA.startTb, metaB.startTs, metaB.startTb);
        });

        mMaxSortedWatermark = watermark;
    }

    void UnionBuffer::compactIfNeeded() {
        if (mRemovedCount == 0) {
            return;
        }

        if (mRowMetas.size() < 10000) {
            return;
        }

        double ratio = static_cast<double>(mRemovedCount) / mRowMetas.size();
        if (ratio >= 0.50) {
            compact();
        }
    }

    void UnionBuffer::compact() {
        if (mRemovedCount == 0) {
            return;
        }

        std::vector<RowMeta> newMetas;
        std::vector<std::uint64_t> newIndex;

        const std::size_t estimated = mRowMetas.size() - mRemovedCount;
        newMetas.reserve(estimated);
        newIndex.reserve(mRowIndex.size());

        for (std::uint64_t idx : mRowIndex) {
            if (idx >= mRowMetas.size()) {
                continue;
            }

            if (mRowMetas[idx].isRemoved) {
                continue;
            }

            const std::size_t newIdx = newMetas.size();
            newMetas.push_back(std::move(mRowMetas[idx]));
            newIndex.push_back(newIdx);
        }

        mRowMetas = std::move(newMetas);
        mRowIndex = std::move(newIndex);

        mRemovedCount = 0;
        //mMaxSortedWatermark = 0;
    }
}
