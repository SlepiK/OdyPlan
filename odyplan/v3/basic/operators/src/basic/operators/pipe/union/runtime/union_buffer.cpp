#include <basic/operators/pipe/union/runtime/union_buffer.h>

#include <algorithm>
#include <iostream>

namespace sierra::odyplan::v3::basic::operators::pipe::union_::runtime {// NOLINT(readability-identifier-naming)
    UnionBuffer::UnionBuffer(const bool outOfOrder,
                             core::logical::port::Port schema,
                             const core::runtime::types::RuntimeTypeRegistry *runtimeTypeRegistry)
        : mOutOfOrder(outOfOrder)
          , mRuntimeTypeRegistry(runtimeTypeRegistry)
          , mLayout(std::move(schema), *runtimeTypeRegistry)
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

    void UnionBuffer::push(const std::size_t index, const void* data) {
        mLayout.push(index, data);
    }

    void UnionBuffer::end(const core::shared::identifiers::PortId inputPort,
                      const core::shared::types::Timestamp startTs, const core::shared::types::Timestamp endTs,
                      const core::shared::types::TiBreak startTb, const core::shared::types::TiBreak endTb) {
        if (!mOutOfOrder) {
            return;
        }

        const std::size_t rowIndex = mRowMetas.size();
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
                                   [this](const std::size_t idx, const std::pair<core::shared::types::Timestamp, core::shared::types::TiBreak>& val) {
                                       return compareRows(mRowMetas[idx].startTs, mRowMetas[idx].startTb, val.first, val.second);
                                   });
        mRowIndex.insert(it, rowIndex);

        mActiveRowCount++;
    }

    std::size_t UnionBuffer::getCount() const {
        return mActiveRowCount;
    }

    bool UnionBuffer::isEmpty() const {
        return mActiveRowCount == 0;
    }

    core::shared::types::Timestamp UnionBuffer::getMinStartTimestamp() const {
        if (mRowIndex.empty()) {
            return core::shared::types::INFINITY_TIMESTAMP_SENTINEL;
        }

        for (const auto idx : mRowIndex) {
            if (idx < mRowMetas.size() && !mRowMetas[idx].isRemoved) {
                return mRowMetas[idx].startTs;
            }
        }

        return core::shared::types::INFINITY_TIMESTAMP_SENTINEL;
    }

    const std::byte* UnionBuffer::getData(const std::size_t index) const {
        if (index >= mRowMetas.size() || mRowMetas[index].isRemoved) {
            return nullptr;
        }
        return mRowMetas[index].values.data();
    }

    const std::byte* UnionBuffer::getFieldPtr(const std::size_t rowIndex, const std::size_t fieldIndex) const {
        if (rowIndex >= mRowMetas.size() || mRowMetas[rowIndex].isRemoved) {
            return nullptr;
        }

        const std::size_t offset = mLayout.offsetOf(fieldIndex);
        return mRowMetas[rowIndex].values.data() + offset;
    }

    core::shared::types::Timestamp UnionBuffer::getStartTimestamp(const std::size_t index) const {
        if (index >= mRowMetas.size()) {
            return core::shared::types::INFINITY_TIMESTAMP_SENTINEL;
        }
        return mRowMetas[index].startTs;
    }

    core::shared::types::Timestamp UnionBuffer::getEndTimestamp(const std::size_t index) const {
        if (index >= mRowMetas.size()) {
            return core::shared::types::INFINITY_TIMESTAMP_SENTINEL;
        }
        return mRowMetas[index].endTs;
    }

    core::shared::types::TiBreak UnionBuffer::getStartTibreaker(const std::size_t index) const {
        if (index >= mRowMetas.size()) {
            return core::shared::types::INFINITY_TIMESTAMP_SENTINEL;
        }
        return mRowMetas[index].startTb;
    }

    core::shared::types::TiBreak UnionBuffer::getEndTibreaker(const std::size_t index) const {
        if (index >= mRowMetas.size()) {
            return core::shared::types::INFINITY_TIMESTAMP_SENTINEL;
        }
        return mRowMetas[index].endTb;
    }

    core::shared::identifiers::PortId UnionBuffer::getInputPort(const std::size_t index) const {
        if (index >= mRowMetas.size()) {
            return 0;
        }
        return mRowMetas[index].inputPort;
    }

    void UnionBuffer::removeV(const std::size_t* indices, const std::size_t count) {
        std::size_t actuallyRemoved = 0;

        for (std::int64_t i = 0; i < count; ++i) {
            const auto index = indices[i];

            if (index < mRowMetas.size() && !mRowMetas[index].isRemoved) {
                mLayout.releaseRow(mRowMetas[index].values);
                mRowMetas[index].isRemoved = true;
                actuallyRemoved++;
                mRemovedCount++;
            }
        }

        if (actuallyRemoved > 0 && mActiveRowCount >= actuallyRemoved) {
            mActiveRowCount -= actuallyRemoved;
        }

        std::vector<std::size_t> newRowIndex;
        newRowIndex.reserve(mRowIndex.size());
        for (auto idx : mRowIndex) {
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
        mLastEmittedWatermark = 0;
        mRemovedCount = 0;
    }

    std::size_t UnionBuffer::getFieldCount() const {
        return mLayout.getFieldCount();
    }

    core::shared::types::ValueTypeId UnionBuffer::getFieldTypeId(const std::size_t index) const {
        return mLayout.getFieldTypeId(index);
    }

    std::size_t UnionBuffer::getFieldOffset(const std::size_t index) const {
        return mLayout.offsetOf(index);
    }

    std::size_t UnionBuffer::getRowByteSize() const {
        return mLayout.getTotalByteSize();
    }

    std::optional<core::shared::types::Timestamp> UnionBuffer::getMinStartTimestampIndex(const core::shared::types::Timestamp watermark) const {
        if (mRowMetas.empty()) {
            return std::nullopt;
        }

        for (auto idx : mRowIndex) {
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

    UnionBuffer::BatchResult UnionBuffer::getReadyIndices(const core::shared::types::Timestamp watermark, const std::size_t maxCount) {
        if (mActiveRowCount == 0) {
            return BatchResult{.count = 0, .indices = mBatchBuffer.data()};
        }

        const auto max = std::min(maxCount, MAX_BATCH_SIZE);
        std::size_t foundCount = 0;

        for (const auto idx : mRowIndex) {
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

    void UnionBuffer::drainEmittedEntries(const core::shared::types::Timestamp watermark) {
        if (watermark <= mLastEmittedWatermark) {
            return;
        }

        std::size_t writeIdx = 0;

        for (std::size_t readIdx = 0; readIdx < mRowIndex.size(); ++readIdx) {
            auto idx = mRowIndex[readIdx];
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
        std::vector<std::size_t> newIndex;

        const std::size_t estimated = mRowMetas.size() - mRemovedCount;
        newMetas.reserve(estimated);
        newIndex.reserve(mRowIndex.size());

        for (auto idx : mRowIndex) {
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
    }
}
