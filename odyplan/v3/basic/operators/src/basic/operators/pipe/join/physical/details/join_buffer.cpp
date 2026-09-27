#include <basic/operators/pipe/join/physical/details/join_buffer.h>
#include <cassert>
#include <cstring>

namespace sierra::odyplan::v3::basic::operators::pipe::join::physical::details {
    JoinBuffer::JoinBuffer(core::logical::port::Port leftSchema,
                           core::logical::port::Port rightSchema,
                           const core::runtime::types::RuntimeTypeRegistry* runtimeTypeRegistry)
        : mLeftSide(std::move(leftSchema), runtimeTypeRegistry),
            mRightSide(std::move(rightSchema), runtimeTypeRegistry),
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

    void JoinBuffer::matchLeft(const Timestamp startTs, const Timestamp endTs, const std::int64_t startTb, const std::int64_t endTb) {
        mLeftSide.setPending(startTs, endTs, startTb, endTb);
        matchAgainst(mLeftSide, mRightSide, mLeftSide.getKeyFieldIndices(), startTs, endTs, false);
    }

    void JoinBuffer::matchRight(const Timestamp startTs, const Timestamp endTs, const std::int64_t startTb, const std::int64_t endTb) {
        mRightSide.setPending(startTs, endTs, startTb, endTb);
        matchAgainst(mRightSide, mLeftSide, mRightSide.getKeyFieldIndices(), startTs, endTs, false);
    }

    void JoinBuffer::matchAgainst(const JoinSide &sourceSide, JoinSide &otherSide,
        const std::vector<std::uint64_t>& sourceKeyFieldIndices, const Timestamp sourceStartTs, const Timestamp sourceEndTs, const bool extract) {
        mCandidates.clear();

        if (sourceSide.hasKeyIndex() && otherSide.hasKeyIndex()) {
            // HASH
            std::vector<const std::byte*> qFieldPtrs;
            for (const auto fieldIndex : sourceKeyFieldIndices) {
                qFieldPtrs.push_back(sourceSide.getLayoutFieldPtr(fieldIndex));
            }
            for (const auto candidate : otherSide.hashLookup(qFieldPtrs)) {
                if (otherSide.overlaps(sourceStartTs, sourceEndTs, candidate)) {
                    mCandidates.push_back(candidate);
                }
            }
        } else {
            // DEFAULT
            const auto size = otherSide.getSize();
            const auto base = otherSide.getBaseIndex();
            for (std::size_t i = base; i < base + size; ++i) {
                if (otherSide.overlaps(sourceStartTs, sourceEndTs, i)) {
                    mCandidates.push_back(i);
                }
            }
        }
    }

    void JoinBuffer::markAllCandidateAsMatch(JoinSnapshot *snapshot, bool extract) {
        if (snapshot == nullptr) {
            return;
        }

        for (std::size_t i = 0; i < snapshot->candidateCount; ++i) {
            snapshot->matchIndicies.push_back(i);

            if (extract) {
                snapshot->markedForRemove.push_back(i);
            }
        }
    }

    void JoinBuffer::endLeft(const Timestamp startTs, const Timestamp endTs, const std::int64_t startTb, const std::int64_t endTb) {
        mLeftSide.end(startTs, endTs, startTb, endTb);
    }

    void JoinBuffer::endRight(const Timestamp startTs, const Timestamp endTs, const std::int64_t startTb, const std::int64_t endTb) {
        mRightSide.end(startTs, endTs, startTb, endTb);
    }

    void JoinBuffer::evict(const Timestamp threshold, const bool callFromLeft, const bool isFinal) {
        if (callFromLeft) {
            mRightSide.evict(threshold, isFinal);
        } else {
            mLeftSide.evict(threshold, isFinal);
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

    void JoinBuffer::markCandidateForRemove(JoinSnapshot *snapshot, const std::size_t candidateIdx) {
        if (snapshot != nullptr && snapshot->isExtraction) {
            snapshot->markedForRemove.push_back(candidateIdx);
        }
    }

    void JoinBuffer::markCandidateAsMatch(JoinSnapshot *snapshot, const std::size_t candidateIdx) {
        if (snapshot != nullptr) {
            snapshot->matchIndicies.push_back(candidateIdx);
        }
    }

    void JoinBuffer::removeMarkedCandidates(JoinSnapshot *snapshot, bool wasLeft) {
        if (snapshot == nullptr || snapshot->markedForRemove.empty()) {
            return;
        }

        auto& otherSide = wasLeft ? mRightSide : mLeftSide;
        otherSide.removeV(mCandidates);
    }

    JoinBuffer::JoinSnapshot *JoinBuffer::snapshotForCompute(const bool callFromLeft, const bool extract) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto* snapshot = new JoinSnapshot();
        snapshot->wasLeft = callFromLeft;
        snapshot->runtimeTypeRegistry = mRuntimeTypeRegistry;

        const auto& side = callFromLeft ? mLeftSide : mRightSide;
        auto& otherSide = callFromLeft ? mRightSide : mLeftSide;

        const std::byte* rowPtr = side.getLayoutRowPtr();
        const std::size_t rowSize = side.getCurrentLayoutRowSize();
        snapshot->data.resize(rowSize);
        std::memcpy(snapshot->data.data(), rowPtr, rowSize);

        snapshot->startTs = side.getLayoutStartTs();
        snapshot->endTs = side.getLayoutEndTs();
        snapshot->startTb = side.getLayoutStartTb();
        snapshot->endTb = side.getLayoutEndTb();

        const std::size_t fieldCount = side.getFieldCount();
        snapshot->fieldOffsets.resize(fieldCount);
        snapshot->fieldTypeIds.resize(fieldCount);

        for (std::size_t p = 0; p < fieldCount; ++p) {
            snapshot->fieldOffsets[p] = side.getFieldOffset(p);
            snapshot->fieldTypeIds[p] = side.getFieldTypeId(p);

            if (!mRuntimeTypeRegistry->isPointerType(snapshot->fieldTypeIds[p])) {
                continue;
            }

            void* oriPtr = nullptr;
            std::memcpy(&oriPtr, &snapshot->data[snapshot->fieldOffsets[p]], sizeof(void*));
            void* retainedPtr = mRuntimeTypeRegistry->retain(snapshot->fieldTypeIds[p], oriPtr);
            std::memcpy(&snapshot->data[snapshot->fieldOffsets[p]], &retainedPtr, sizeof(void*));
            snapshot->retainPointers.push_back({.typeId = snapshot->fieldTypeIds[p], .ptr = retainedPtr});
        }

        snapshot->candidateRowByteSize = otherSide.getRowByteSize();
        snapshot->candidateCount = mCandidates.size();
        snapshot->candidateData.resize(mCandidates.size() * snapshot->candidateRowByteSize);
        snapshot->candidateStarts.resize(mCandidates.size());
        snapshot->candidateEnds.resize(mCandidates.size());
        snapshot->candidateStartsTb.resize(mCandidates.size());
        snapshot->candidateEndsTb.resize(mCandidates.size());

        for (std::size_t i = 0; i < mCandidates.size(); ++i) {
            const auto rowIndex = mCandidates[i];
            snapshot->candidateStarts[i] = otherSide.getStartTimestamp(rowIndex);
            snapshot->candidateEnds[i] = otherSide.getEndTimestamp(rowIndex);
            snapshot->candidateStartsTb[i] = otherSide.getStartTibreaker(rowIndex);
            snapshot->candidateEndsTb[i] = otherSide.getEndTibreaker(rowIndex);

            copyRowIntoSnapshot(otherSide, rowIndex, snapshot->candidateData, i * snapshot->candidateRowByteSize,
                                snapshot->candidateFieldOffsets, snapshot->candidateFieldTypeIds, snapshot->retainPointers);
        }

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