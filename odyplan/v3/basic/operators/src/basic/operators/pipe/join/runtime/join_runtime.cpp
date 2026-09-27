#include <atomic>
#include <basic/operators/pipe/join/runtime/join_runtime.h>

namespace sierra::odyplan::v3::basic::operators::pipe::join::runtime {
    namespace details {
        struct JoinWatermarkState {
            std::atomic_flag lock = ATOMIC_FLAG_INIT;
            core::shared::types::Timestamp leftWatermark{0};
            core::shared::types::Timestamp rightWatermark{0};
            bool leftReported{false};
            bool rightReported{false};
            bool leftFinal{false};
            bool rightFinal{false};
            bool finalEmitted = false;
            core::shared::types::Timestamp lastEmittedWatermark{0};
        };
    }

    extern "C" {
    // NOLINTNEXTLINE(readability-function-size)
    void *rtJoinCreate(const void *typeRegistryRawPtr,
                       const core::shared::types::ValueTypeId *leftTypeIds,
                       const std::uint64_t leftFieldSize,
                       const core::shared::types::ValueTypeId *rightTypeIds,
                       const std::uint64_t rightFieldSize,
                       const std::uint8_t hasEquiKey,
                       const std::uint64_t *leftKeyFieldIndices,
                       const std::uint64_t *rightKeyFieldIndices,
                       const core::shared::types::ValueTypeId *keyTypeIds,
                       const std::uint64_t equiKeyCnt,
                       const uint8_t outOfOrder) {
        const auto *runtimeTypeRegistry =
                static_cast<const core::runtime::types::RuntimeTypeRegistry *>(typeRegistryRawPtr);

        core::logical::port::Port leftSchema;
        core::logical::port::Port rightSchema;
        core::logical::port::Port outputSchema;

        for (std::uint64_t i = 0; i < leftFieldSize; ++i) {
            leftSchema.push_back(
                core::logical::port::PortDescription{.valueTypeId = leftTypeIds[i]}
            );
            outputSchema.push_back(
                core::logical::port::PortDescription{.valueTypeId = leftTypeIds[i]}
            );
        }

        for (std::uint64_t i = 0; i < rightFieldSize; ++i) {
            rightSchema.push_back(
                core::logical::port::PortDescription{.valueTypeId = rightTypeIds[i]}
            );
            outputSchema.push_back(
                core::logical::port::PortDescription{.valueTypeId = rightTypeIds[i]}
            );
        }

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto *buffer =
                new JoinBuffer(std::move(leftSchema), std::move(rightSchema),
                                                  std::move(outputSchema), runtimeTypeRegistry, outOfOrder != 0);

        if (hasEquiKey != 0) {
            std::vector<std::size_t> leftIndices;
            std::vector<std::size_t> rightIndices;
            std::vector<core::shared::types::ValueTypeId> typeIds;

            leftIndices.reserve(equiKeyCnt);
            rightIndices.reserve(equiKeyCnt);
            typeIds.reserve(equiKeyCnt);

            for (std::uint64_t i = 0; i < equiKeyCnt; ++i) {
                leftIndices.push_back(static_cast<std::size_t>(leftKeyFieldIndices[i]));
                rightIndices.push_back(static_cast<std::size_t>(rightKeyFieldIndices[i]));
                typeIds.push_back(keyTypeIds[i]);
            }

            buffer->enableEqui(leftIndices, rightIndices, typeIds);
        }

        return buffer;
    }

    void rtJoinDestroy(void *state) {
        auto *buffer = static_cast<JoinBuffer *>(state);

        buffer->close();
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete buffer;
    }

    void rtJoinLock(void *state) {
        static_cast<JoinBuffer *>(state)->lock();
    }

    void rtJoinUnlock(void *state) {
        static_cast<JoinBuffer *>(state)->unlock();
    }

    void rtJoinBeginLeft(void *state) {
        static_cast<JoinBuffer *>(state)->beginLeft();
    }

    void rtJoinBeginRight(void *state) {
        static_cast<JoinBuffer *>(state)->beginRight();
    }

    void rtJoinPushLeft(void *state, const std::uint64_t index, const void *data) {
        static_cast<JoinBuffer *>(state)->pushLeft(index, data);
    }

    void rtJoinPushRight(void *state, const std::uint64_t index, const void *data) {
        static_cast<JoinBuffer *>(state)->pushRight(index, data);
    }

    void rtJoinEndLeft(void *state, const core::shared::types::Timestamp startTs, const core::shared::types::Timestamp endTs, const core::shared::types::TiBreak startTb,
                       const core::shared::types::TiBreak endTb) {
        static_cast<JoinBuffer *>(state)->endLeft(startTs, endTs, startTb, endTb);
    }

    void rtJoinEndRight(void *state, const core::shared::types::Timestamp startTs, const core::shared::types::Timestamp endTs, const core::shared::types::TiBreak startTb,
                        const core::shared::types::TiBreak endTb) {
        static_cast<JoinBuffer *>(state)->endRight(startTs, endTs, startTb, endTb);
    }

    void *rtJoinSnapshotForCompute(void *state, const uint8_t callFromLeft, const uint8_t extract) {
        return static_cast<JoinBuffer *>(state)->snapshotForCompute(callFromLeft != 0, extract != 0);
    }

    const void *rtJoinSnapshotGetOwnPtr(void *snapshot, const std::uint64_t fieldIndex) {
        const auto *s = static_cast<JoinBuffer::JoinSnapshot *>(snapshot);
        return s->ownFieldPtr(static_cast<std::size_t>(fieldIndex));
    }

    const void *rtJoinSnapshotGetCandidatePtr(void *snapshot, const std::uint64_t index,
                                              const std::uint64_t fieldIndex) {
        auto *s = static_cast<JoinBuffer::JoinSnapshot *>(snapshot);
        return s->candidateFieldPtr(static_cast<std::size_t>(index), static_cast<std::size_t>(fieldIndex));
    }

    core::shared::types::Timestamp rtJoinSnapshotGetCandidateStartTimestamp(void *snapshot, const std::uint64_t index) {
        return static_cast<JoinBuffer::JoinSnapshot *>(snapshot)->candidateStarts.at(index);
    }

    core::shared::types::Timestamp rtJoinSnapshotGetCandidateEndTimestamp(void *snapshot, const std::uint64_t index) {
        return static_cast<JoinBuffer::JoinSnapshot *>(snapshot)->candidateEnds.at(index);
    }

    core::shared::types::Timestamp rtJoinSnapshotGetOwnStartTimestamp(void *snapshot) {
        return static_cast<JoinBuffer::JoinSnapshot *>(snapshot)->startTs;
    }

    core::shared::types::TiBreak rtJoinSnapshotGetCandidateStartTibreaker(void *snapshot, const std::uint64_t index) {
        return static_cast<JoinBuffer::JoinSnapshot *>(snapshot)->candidateStartsTb.at(index);
    }

    core::shared::types::TiBreak rtJoinSnapshotGetCandidateEndTibreaker(void *snapshot, const std::uint64_t index) {
        return static_cast<JoinBuffer::JoinSnapshot *>(snapshot)->candidateEndsTb.at(index);
    }

    core::shared::types::TiBreak rtJoinSnapshotGetOwnStartTibreaker(void *snapshot) {
        return static_cast<JoinBuffer::JoinSnapshot *>(snapshot)->startTb;
    }

    core::shared::types::TiBreak rtJoinSnapshotGetOwnEndTibreaker(void *snapshot) {
        return static_cast<JoinBuffer::JoinSnapshot *>(snapshot)->endTb;
    }

    core::shared::types::Timestamp rtJoinSnapshotGetOwnEndTimestamp(void *snapshot) {
        return static_cast<JoinBuffer::JoinSnapshot *>(snapshot)->endTs;
    }

    std::uint64_t rtJoinSnapshotGetCount(void *snapshot) {
        auto *s = static_cast<JoinBuffer::JoinSnapshot *>(snapshot);
        return static_cast<std::uint64_t>(s->candidateCount);
    }

    void rtJoinSnapshotRelease(void *snapshot) {
        JoinBuffer::releaseSnapshot(
            static_cast<JoinBuffer::JoinSnapshot *>(snapshot));
    }

    bool rtJoinSnapshotGetWasLeft(void *snapshot) {
        return static_cast<JoinBuffer::JoinSnapshot *>(snapshot)->wasLeft;
    }

    void *rtJoinWatermarkCreate(void * /*typeRegistryRawPtr*/) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        return new details::JoinWatermarkState();
    }

    void rtJoinWatermarkDestroy(void *state) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete static_cast<details::JoinWatermarkState *>(state);
    }

    core::shared::types::Timestamp rtJoinGetGlobalWatermark(void* state) {
        auto* watermark = static_cast<details::JoinWatermarkState*>(state);
        if (!watermark->leftReported || !watermark->rightReported) {
            return core::shared::types::INFINITY_TIMESTAMP_SENTINEL;
        }
        return std::min(watermark->leftWatermark, watermark->rightWatermark);
    }

    void rtJoinMarkCandidateForRemove(void *state, void *snapshot, const std::uint64_t candidateIdx) {
        auto *joinBuffer = static_cast<JoinBuffer *>(state);
        auto *snap = static_cast<JoinBuffer::JoinSnapshot *>(snapshot);
        joinBuffer->markCandidateForRemove(snap, candidateIdx);
    }

    void rtJoinMarkCandidateAsMatch(void *state, void *snapshot, const std::uint64_t candidateIdx) {
        auto *joinBuffer = static_cast<JoinBuffer *>(state);
        auto *snap = static_cast<JoinBuffer::JoinSnapshot *>(snapshot);
        joinBuffer->markCandidateAsMatch(snap, candidateIdx);
    }

    void rtJoinMarkAllCandidatesAsMatch(void *state, void *snapshot, const std::uint8_t extract) {
        auto *joinBuffer = static_cast<JoinBuffer *>(state);
        auto *snap = static_cast<JoinBuffer::JoinSnapshot *>(snapshot);
        joinBuffer->markAllCandidateAsMatch(snap, extract != 0);
    }

    void rtJoinRemoveMarkedCandidates(void *state, void *snapshot) {
        auto *joinBuffer = static_cast<JoinBuffer *>(state);
        auto *snap = static_cast<JoinBuffer::JoinSnapshot *>(snapshot);
        joinBuffer->removeMarkedCandidates(snap, snap->wasLeft);
    }

    std::uint64_t rtJoinSnapshotGetMatchCount(void *snapshot) {
        const auto *snap = static_cast<JoinBuffer::JoinSnapshot *>(snapshot);
        return snap != nullptr ? snap->matchIndicies.size() : 0;
    }

    std::uint64_t rtJoinSnapshotGetMatchIndex(void *snapshot, const std::uint64_t matchIndex) {
        const auto *snap = static_cast<JoinBuffer::JoinSnapshot *>(snapshot);
        if (snap == nullptr || matchIndex >= snap->matchIndicies.size()) {
            return 0;
        }
        return snap->matchIndicies.at(matchIndex);
    }

    // NOLINTNEXTLINE(readability-function-size)
    std::uint8_t rtJoinUpdateWatermark(void *state, const std::uint8_t isLeft, const core::shared::types::Timestamp inWatermark,
                                       const std::uint8_t inIsFinal, const core::shared::types::Timestamp allowedLateness,
                                       std::int64_t *outEvictLeftTh, std::int64_t *outEvictRightTh,
                                       std::int64_t *outGlobalWatermark, std::uint8_t *outIsFinal) {
        auto *watermarkState = static_cast<details::JoinWatermarkState *>(state);

        while (watermarkState->lock.test_and_set(std::memory_order_acquire)) {
        }

        if (isLeft != 0) {
            watermarkState->leftWatermark = watermarkState->leftReported
                                                ? std::max(watermarkState->leftWatermark, inWatermark)
                                                : inWatermark;
            watermarkState->leftReported = true;
            if (inIsFinal != 0) {
                watermarkState->leftFinal = true;
            }
        } else {
            watermarkState->rightWatermark = watermarkState->rightReported
                                                 ? std::max(watermarkState->rightWatermark, inWatermark)
                                                 : inWatermark;
            watermarkState->rightReported = true;
            if (inIsFinal != 0) {
                watermarkState->rightFinal = true;
            }
        }

        std::uint8_t shouldEmit = 0;

        if (watermarkState->leftFinal && watermarkState->rightFinal) {
            *outEvictLeftTh = std::numeric_limits<std::int64_t>::max();
            *outEvictRightTh = std::numeric_limits<std::int64_t>::max();

            if (!watermarkState->finalEmitted) {
                watermarkState->finalEmitted = true;
                *outGlobalWatermark = std::numeric_limits<std::int64_t>::max();
                *outIsFinal = 1;
                shouldEmit = 1;
            }
        } else {
            *outEvictLeftTh = watermarkState->rightReported
                                  ? watermarkState->rightWatermark - allowedLateness
                                  : std::numeric_limits<std::int64_t>::min();
            *outEvictRightTh = watermarkState->leftReported
                                   ? watermarkState->leftWatermark - allowedLateness
                                   : std::numeric_limits<std::int64_t>::min();

            if (watermarkState->leftReported && watermarkState->rightReported) {
                const auto minWatermark = std::min(watermarkState->leftWatermark, watermarkState->rightWatermark);
                if (minWatermark > watermarkState->lastEmittedWatermark) {
                    watermarkState->lastEmittedWatermark = minWatermark;
                    *outGlobalWatermark = minWatermark;
                    *outIsFinal = 0;
                    shouldEmit = 1;
                }
            }
        }

        watermarkState->lock.clear(std::memory_order_release);
        return shouldEmit;
    }

    void rtJoinEvict(void *state, const core::shared::types::Timestamp threshold, const std::uint8_t callFromLeft,
                     const std::uint8_t isFinal) {
        static_cast<JoinBuffer *>(state)->evict(threshold, callFromLeft != 0, isFinal != 0);
    }

    void rtJoinMatchLeft(void *state, const core::shared::types::Timestamp startTs, const core::shared::types::Timestamp endTs, const core::shared::types::TiBreak startTb,
                         const core::shared::types::TiBreak endTb) {
        static_cast<JoinBuffer *>(state)->matchLeft(startTs, endTs, startTb, endTb);
    }

    void rtJoinMatchRight(void *state, const core::shared::types::Timestamp startTs, const core::shared::types::Timestamp endTs, const core::shared::types::TiBreak startTb,
                          const core::shared::types::TiBreak endTb) {
        static_cast<JoinBuffer *>(state)->matchRight(startTs, endTs, startTb, endTb);
    }

    void rtJoinBufferOutput(void *state, const void *ownRowPtr, const std::uint64_t ownRowSize, const void *candRowPtr,
                            const std::uint64_t candRowSize, const std::uint8_t onLeft, const core::shared::types::Timestamp startTs,
                            const core::shared::types::Timestamp endTs,
                            const core::shared::types::TiBreak startTb, const core::shared::types::TiBreak endTb) {
        auto *buffer = static_cast<JoinBuffer *>(state);
        buffer->bufferOutput(ownRowPtr, ownRowSize, candRowPtr, candRowSize, onLeft != 0, startTs, endTs, startTb,
                             endTb);
    }

    std::uint64_t rtJoinDrainOutput(void *state, const core::shared::types::Timestamp watermark, const std::uint64_t maxCount,
                                    std::uint64_t *outIndices) {
        auto *buffer = static_cast<JoinBuffer *>(state);
        std::vector<std::size_t> indices;
        auto count = buffer->drainOutput(watermark, maxCount, indices);
        for (std::size_t i = 0; i < indices.size(); ++i) {
            outIndices[i] = indices[i];
        }
        return count;
    }

    const void *rtJoinGetOutputRowPtr(void *state, const std::uint64_t index) {
        return static_cast<JoinBuffer *>(state)->getOutputRow(index).data.data();
    }

    core::shared::types::Timestamp rtJoinGetOutputRowStartTs(void *state, const std::uint64_t index) {
        return static_cast<JoinBuffer *>(state)->getOutputRow(index).startTs;
    }

    core::shared::types::Timestamp rtJoinGetOutputRowEndTs(void *state, const std::uint64_t index) {
        return static_cast<JoinBuffer *>(state)->getOutputRow(index).endTs;
    }

    core::shared::types::TiBreak rtJoinGetOutputRowStartTb(void *state, const std::uint64_t index) {
        return static_cast<JoinBuffer *>(state)->getOutputRow(index).startTb;
    }

    core::shared::types::TiBreak rtJoinGetOutputRowEndTb(void *state, const std::uint64_t index) {
        return static_cast<JoinBuffer *>(state)->getOutputRow(index).endTb;
    }

    std::uint64_t rtJoinGetOutputRowSize(void *state) {
        return static_cast<JoinBuffer *>(state)->getOutputRowSize();
    }

    const void *rtJoinSnapshotGetOwnRowPtr(void *snapshot) {
        return static_cast<JoinBuffer::JoinSnapshot *>(snapshot)->data.data();
    }

    std::uint64_t rtJoinSnapshotGetOwnRowSize(void *snapshot) {
        return static_cast<JoinBuffer::JoinSnapshot *>(snapshot)->data.size();
    }

    const void *rtJoinSnapshotGetCandidateRowPtr(void *snapshot, const std::uint64_t index) {
        auto *s = static_cast<JoinBuffer::JoinSnapshot *>(snapshot);
        return s->candidateData.data() + (index * s->candidateRowByteSize);
    }

    std::uint64_t rtJoinSnapshotGetCandidateRowSize(void *snapshot) {
        return static_cast<std::uint64_t>(static_cast<JoinBuffer::JoinSnapshot *>(snapshot)->
            candidateRowByteSize);
    }

    void rtJoinRemoveOutputRows(void* state, const std::uint64_t* indices, std::uint64_t count) {
        auto* buf = static_cast<JoinBuffer*>(state);
        const std::vector<std::size_t> idx(indices, indices + count);
        buf->removeOutputRows(idx);
    }

    core::shared::types::Timestamp rtJoinGetMinStartTimestampLeft(void* state) {
        return static_cast<JoinBuffer*>(state)->getMinStartTimestampLeft();
    }

    core::shared::types::Timestamp rtJoinGetMinStartTimestampRight(void* state) {
        return static_cast<JoinBuffer*>(state)->getMaxStartTimestampRight();
    }
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::runtime