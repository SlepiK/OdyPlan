#include <basic/operators/pipe/union/runtime/union_buffer.h>

namespace sierra::odyplan::v3::basic::operators::pipe::union_::runtime {// NOLINT(readability-identifier-naming)
    UnionBuffer::UnionBuffer(const bool outOfOrder, core::logical::port::Port schema, const core::runtime::types::RuntimeTypeRegistry *runtimeTypeRegistry)
        : mOutOfOrder(outOfOrder),
        mRuntimeTypeRegistry(runtimeTypeRegistry),
        mRowLayout(std::move(schema), *runtimeTypeRegistry), mActiveRowCount(0) {
    }

    UnionBuffer::~UnionBuffer() {
        clear();
    }

    void UnionBuffer::lock() {
        mMutex.lock();
    }

    void UnionBuffer::unlock() {
        mMutex.unlock();
    }

    void UnionBuffer::begin() {
        mRowLayout.begin();
    }

    void UnionBuffer::push(const std::uint64_t index, const void *data) {
        mRowLayout.push(index, data);
    }

    void UnionBuffer::end(const core::shared::identifiers::PortId inputPort, const Timestamp startTs, const Timestamp endTs, const std::int64_t startTb, const std::int64_t endTb) {
        mRows.push_back(
                    RowBuffer{
                        .values = mRowLayout.pop(),
                        .inputPort = inputPort,
                        .startTs = startTs,
                        .endTs = endTs,
                        .startTb = startTb,
                        .endTb = endTb,
                        .isRemoved = false,
                        .rowIndex = mRows.size()
                    });
        mActiveRowCount++;

        if (mOutOfOrder) {
            mRowIndex.push(mRows.back());
        }
    }

    std::uint64_t UnionBuffer::getCount() const {
        return mActiveRowCount;
    }

    bool UnionBuffer::isEmpty() const {
        return mActiveRowCount == 0;
    }

    Timestamp UnionBuffer::getMinStartTimestamp() {
        if (mRowIndex.empty()) {
            Timestamp min = 0;
            bool found = false;
            for (const auto& row : mRows) {
                if (!row.isRemoved) {
                    if (!found || row.startTs < min) {
                        min = row.startTs;
                        found = true;
                    }
                }
            }
            return found ? min : core::physical::time::INFINITY_SENTINEL;
        }

        while (!mRowIndex.empty()) {
            const auto& top = mRowIndex.top();
            if (!top.isRemoved) {
                return top.startTs;
            }
            mRowIndex.pop();
        }

        return core::physical::time::INFINITY_SENTINEL;
    }

    const std::byte *UnionBuffer::getData(const std::uint64_t index) const {
        if (index >= mRows.size() || mRows[index].isRemoved) {
            return nullptr;
        }
        return mRows[index].values.data();
    }

    std::optional<std::uint64_t> UnionBuffer::getMinStartTimestampIndex(const Timestamp watermark) {
        if (mRows.empty()) {
            return std::nullopt;
        }

        while (!mRowIndex.empty()) {
            const auto& top = mRowIndex.top();

            if (top.isRemoved) {
                mRowIndex.pop();
                continue;
            }

            if (top.rowIndex >= mRows.size() || mRows[top.rowIndex].isRemoved) {
                mRowIndex.pop();
                continue;
            }

            if (top.startTs > watermark) {
                return std::nullopt;
            }

            return top.rowIndex;
        }

        return std::nullopt;
    }

    const std::byte *UnionBuffer::getFieldPtr(const std::uint64_t rowIndex, const std::uint64_t fieldIndex) const {
        const std::byte* data = getData(rowIndex);
        if (data == nullptr) {
            return nullptr;
        }

        const std::size_t offset = mRowLayout.offsetOf(fieldIndex);
        return data + offset;
    }

    Timestamp UnionBuffer::getStartTimestamp(const std::uint64_t index) const {
        if (index >= mRows.size()) {
            return core::physical::time::INFINITY_SENTINEL;
        }
        return mRows[index].startTs;
    }

    Timestamp UnionBuffer::getEndTimestamp(const std::uint64_t index) const {
        if (index >= mRows.size()) {
            return core::physical::time::INFINITY_SENTINEL;
        }
        return mRows[index].endTs;
    }

    std::int64_t UnionBuffer::getStartTibreaker(const std::uint64_t index) const {
        if (index >= mRows.size()) {
            return core::physical::time::INFINITY_SENTINEL;
        }
        return mRows[index].startTb;
    }

    std::int64_t UnionBuffer::getEndTibreaker(const std::uint64_t index) const {
        if (index >= mRows.size()) {
            return core::physical::time::INFINITY_SENTINEL;
        }
        return mRows[index].endTb;
    }

    core::shared::identifiers::PortId UnionBuffer::getInputPort(const std::uint64_t index) const {
        if (index >= mRows.size()) {
            return 0;
        }
        return mRows[index].inputPort;
    }

    void UnionBuffer::removeV(const std::vector<std::uint64_t> &indices) {
        for (const auto index : indices) {
            if (index < mRows.size() && !mRows[index].isRemoved) {
                mRowLayout.releaseRow(mRows[index].values);
                mRows[index].isRemoved = true;
                if (mActiveRowCount > 0) {
                    mActiveRowCount--;
                }
            }
        }
    }

    void UnionBuffer::clear() {
        for (auto& row : mRows) {
            if (!row.isRemoved) {
                mRowLayout.releaseRow(row.values);
            }
        }
        mRows.clear();
        mActiveRowCount = 0;

        while (!mRowIndex.empty()) {
            mRowIndex.pop();
        }
    }

    std::uint64_t UnionBuffer::getFieldCount() const {
        return mRowLayout.getFieldCount();
    }

    core::shared::types::ValueTypeId UnionBuffer::getFieldTypeId(const std::uint64_t index) const {
        return mRowLayout.getFieldTypeId(index);
    }

    std::uint64_t UnionBuffer::getFieldOffset(const std::uint64_t index) const {
        return mRowLayout.offsetOf(index);
    }

    std::uint64_t UnionBuffer::getRowByteSize() const {
        return mRowLayout.getTotalByteSize();
    }
}
