#include <basic/operators/pipe/join/physical/details/join_buffer.h>
#include <cassert>
#include <cstring>

namespace sierra::odyplan::v3::basic::operators::pipe::join::physical::details {
    JoinBuffer::JoinBuffer(core::logical::port::Port leftSchema,
                           core::logical::port::Port rightSchema,
                           const core::runtime::types::RuntimeTypeRegistry* runtimeTypeRegistry,
                           const std::chrono::milliseconds leftWindowSize,
                           const std::chrono::milliseconds rightWindowSize)
        : mLeftSide(std::move(leftSchema), runtimeTypeRegistry, leftWindowSize),
            mRightSide(std::move(rightSchema), runtimeTypeRegistry, rightWindowSize),
          mRuntimeTypeRegistry(runtimeTypeRegistry) {}

    JoinBuffer::~JoinBuffer() {
        assert(mAllReleased && "JoinBuffer: left and right side was never released!");
    }

    void JoinBuffer::close() {
        mLeftSide.release();
        mRightSide.release();
        mAllReleased = true;
    }

    void JoinBuffer::lock() {
        mMutex.lock();
    }

    void JoinBuffer::unlock() {
        mMutex.unlock();
    }

    void JoinBuffer::beginLeft() {
        mLeftSide.begin();
    }

    void JoinBuffer::beginRight() {
        mRightSide.begin();
    }

    void JoinBuffer::pushLeft(const std::size_t index, const void* data) {
        mLeftSide.push(index, data);
    }

    void JoinBuffer::pushRight(const std::size_t index, const void* data) {
        mRightSide.push(index, data);
    }

    void JoinBuffer::matchAgainst(const JoinSide &sourceSide, JoinSide &otherSide,
        const std::vector<std::size_t>& sourceKeyFieldIndices, const Timestamp timestamp,
        const std::chrono::milliseconds sourceWindowSize) {
        mCandidates.clear();

        if (sourceSide.hasKeyIndex() && otherSide.hasKeyIndex()) {
            // HASH
            const auto newAbsIndex = sourceSide.getLastAbsoluteIndex();
            std::vector<const std::byte*> qFieldPtrs;
            for (const auto fieldIndex : sourceKeyFieldIndices) {
                qFieldPtrs.push_back(sourceSide.getFieldPtr(newAbsIndex, fieldIndex));
            }
            for (const auto candidate : otherSide.hashLookup(qFieldPtrs)) {
                if (otherSide.isOver(timestamp, sourceWindowSize, candidate)) {
                    mCandidates.push_back(candidate);
                }
            }
        } else {
            // DEFAULT
            const auto size = otherSide.getSize();
            const auto base = otherSide.getBaseIndex();
            for (std::size_t i = base; i < base + size; ++i) {
                if (otherSide.isOver(timestamp, sourceWindowSize, i)) {
                    mCandidates.push_back(i);
                }
            }
        }

    }

    void JoinBuffer::endLeft(const Timestamp timestamp) {
        mLeftSide.end(timestamp);
        mCandidateOnRight = true;

        matchAgainst(mLeftSide, mRightSide, mLeftSide.getKeyFieldIndices(), timestamp, mLeftSide.getWindowSize());
    }

    void JoinBuffer::endRight(const Timestamp timestamp) {
        mRightSide.end(timestamp);
        mCandidateOnRight = false;

        matchAgainst(mRightSide, mLeftSide, mRightSide.getKeyFieldIndices(), timestamp, mRightSide.getWindowSize());
    }

    void JoinBuffer::evict(const Timestamp timestamp, const bool callFromLeft) {
        if (callFromLeft) {
            mRightSide.evict(timestamp);
        } else {
            mLeftSide.evict(timestamp);
        }
    }

    void JoinBuffer::copyRowIntoSnapshot(const JoinSide &side, std::uint64_t rowIndex, std::vector<std::byte> &dst, std::size_t dstOffset,
        std::vector<std::size_t> &offsetsOut, std::vector<core::shared::types::ValueTypeId> &typeIdsOut, std::vector<RetainPointer> &retainedOut) const {
        const std::size_t rowSize = side.getRowByteSize();
        const std::byte* rowPtr = side.getRowPtr(rowIndex);

        std::memcpy(&dst[dstOffset], rowPtr, rowSize);

        const std::size_t fieldCount = side.getFieldCount();
        offsetsOut.resize(fieldCount);
        typeIdsOut.resize(fieldCount);

        for (std::size_t p = 0; p < fieldCount; ++p) {
            const auto typeId = side.getFieldTypeId(p);
            const auto offset = side.getFieldOffset(p);
            offsetsOut[p] = offset;
            typeIdsOut[p] = typeId;

            if (!mRuntimeTypeRegistry->isPointerType(typeId)) {
                continue;
            }

            void* oriPtr = nullptr;
            std::memcpy(&oriPtr, &dst[dstOffset + offset], sizeof(void*));

            void* retainedPtr = mRuntimeTypeRegistry->retain(typeId, oriPtr);

            std::memcpy(&dst[dstOffset + offset], &retainedPtr, sizeof(void*));

            retainedOut.push_back({.typeId=typeId, .ptr=retainedPtr});
        }
    }

    JoinBuffer::JoinSnapshot *JoinBuffer::snapshotForComputeAndUnlock(const bool callFromLeft) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto* snapshot = new JoinSnapshot();
        snapshot->wasLeft = callFromLeft;
        snapshot->runtimeTypeRegistry = mRuntimeTypeRegistry;

        auto& side = callFromLeft ? mLeftSide : mRightSide;
        auto& otherSide = callFromLeft ? mRightSide : mLeftSide;

        const auto index = side.getLastAbsoluteIndex();
        snapshot->data.resize(side.getRowByteSize());
        snapshot->timestamp = side.getTimestamp(index);

        copyRowIntoSnapshot(side, index, snapshot->data, 0, snapshot->fieldOffsets, snapshot->fieldTypeIds,
                            snapshot->retainPointers);

        snapshot->candidateRowByteSize = otherSide.getRowByteSize();
        snapshot->candidateCount = mCandidates.size();
        snapshot->candidateData.resize(mCandidates.size() * snapshot->candidateRowByteSize);
        snapshot->candidateTimestamps.resize(mCandidates.size());

        for (std::size_t i = 0; i < mCandidates.size(); ++i) {
            const auto rowIndex = mCandidates[i];
            snapshot->candidateTimestamps[i] = otherSide.getTimestamp(rowIndex);

            copyRowIntoSnapshot(otherSide, rowIndex, snapshot->candidateData, i * snapshot->candidateRowByteSize,
                                snapshot->candidateFieldOffsets, snapshot->candidateFieldTypeIds, snapshot->retainPointers);
        }

        unlock();

        return snapshot;
    }

    void JoinBuffer::releaseSnapshot(JoinSnapshot *snapshot) {
        if (snapshot == nullptr) {
            return;
        }

        for (const auto& [typeId, ptr] : snapshot->retainPointers) {
            snapshot->runtimeTypeRegistry->release(typeId, ptr);
        }

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete snapshot;
    }

    void JoinBuffer::enableEqui(const std::vector<std::size_t>& leftFieldIndices,
                        const std::vector<std::size_t>& rightFieldIndices,
                        const std::vector<core::shared::types::ValueTypeId>& keyTypeIds) {
        mLeftSide.enableKeyIdx(leftFieldIndices, keyTypeIds);
        mRightSide.enableKeyIdx(rightFieldIndices, keyTypeIds);
    }

    [[nodiscard]] const std::byte* JoinBuffer::JoinSnapshot::ownFieldPtr(const std::size_t fieldIdx) const {
        return data.data() + fieldOffsets.at(fieldIdx);
    }

    [[nodiscard]] const std::byte* JoinBuffer::JoinSnapshot::candidateFieldPtr(const std::size_t candidateIdx, const std::size_t fieldIdx) const {
        return candidateData.data() + (candidateIdx * candidateRowByteSize) + candidateFieldOffsets.at(fieldIdx);
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::physical::details