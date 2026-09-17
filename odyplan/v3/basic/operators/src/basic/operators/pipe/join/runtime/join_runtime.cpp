#include <atomic>
#include <basic/operators/pipe/join/runtime/join_runtime.h>

namespace sierra::odyplan::v3::basic::operators::pipe::join::runtime {
    namespace details {
        struct JoinWatermarkState {
            std::atomic_flag lock = ATOMIC_FLAG_INIT;
            std::int64_t leftWatermark{0};
            std::int64_t rightWatermark{0};
            bool leftReported{false};
            bool rightReported{false};
            bool leftFinal{false};
            bool rightFinal{false};
            bool finalEmitted = false;
            std::int64_t lastEmittedWatermark{0};
        };
    }

    extern "C" {
    // NOLINTNEXTLINE(readability-function-size)
    void* rtJoinCreate(const void* typeRegistryRawPtr,
                       const core::shared::types::ValueTypeId* leftTypeIds,
                       const std::uint64_t leftFieldSize,
                       const core::shared::types::ValueTypeId* rightTypeIds,
                       const std::uint64_t rightFieldSize,
                       const std::uint8_t hasEquiKey,
                       const std::uint64_t* leftKeyFieldIndices,
                       const std::uint64_t* rightKeyFieldIndices,
                       const core::shared::types::ValueTypeId* keyTypeIds,
                       const std::uint64_t equiKeyCnt) {
        const auto* runtimeTypeRegistry =
            static_cast<const core::runtime::types::RuntimeTypeRegistry*>(typeRegistryRawPtr);

        core::logical::port::Port leftSchema;
        core::logical::port::Port rightSchema;

        for (std::uint64_t i = 0; i < leftFieldSize; ++i) {
            leftSchema.push_back(
                core::logical::port::PortDescription{.valueTypeId = leftTypeIds[i]});
        }

        for (std::uint64_t i = 0; i < rightFieldSize; ++i) {
            rightSchema.push_back(
                core::logical::port::PortDescription{.valueTypeId = rightTypeIds[i]});
        }

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto* buffer =
            new physical::details::JoinBuffer(std::move(leftSchema), std::move(rightSchema), runtimeTypeRegistry);

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

    void rtJoinDestroy(void* state) {
        auto* buffer = static_cast<physical::details::JoinBuffer*>(state);

        buffer->close();
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete buffer;
    }

    void rtJoinLock(void* state) {
        static_cast<physical::details::JoinBuffer*>(state)->lock();
    }

    void rtJoinUnlock(void* state) {
        static_cast<physical::details::JoinBuffer*>(state)->unlock();
    }

    void rtJoinBeginLeft(void* state) {
        static_cast<physical::details::JoinBuffer*>(state)->beginLeft();
    }

    void rtJoinBeginRight(void* state) {
        static_cast<physical::details::JoinBuffer*>(state)->beginRight();
    }

    void rtJoinPushLeft(void* state, const std::uint64_t index, const void* data) {
        static_cast<physical::details::JoinBuffer*>(state)->pushLeft(index, data);
    }

    void rtJoinPushRight(void* state, const std::uint64_t index, const void* data) {
        static_cast<physical::details::JoinBuffer*>(state)->pushRight(index, data);
    }

    void rtJoinEndLeft(void* state, const std::int64_t startTs, const std::int64_t endTs, const std::int64_t startTb, const std::int64_t endTb) {
        static_cast<physical::details::JoinBuffer*>(state)->endLeft(startTs, endTs, startTb, endTb);
    }

    void rtJoinEndRight(void* state, const std::int64_t startTs, const std::int64_t endTs, const std::int64_t startTb, const std::int64_t endTb) {
        static_cast<physical::details::JoinBuffer*>(state)->endRight(startTs, endTs, startTb, endTb);
    }

    void *rtJoinSnapshotForCompute(void *state, const uint8_t callFromLeft, const uint8_t extract) {
        return static_cast<physical::details::JoinBuffer*>(state)->snapshotForCompute(callFromLeft != 0, extract != 0);
    }

    const void *rtJoinSnapshotGetOwnPtr(void *snapshot, const std::uint64_t fieldIndex) {
        const auto* s = static_cast<physical::details::JoinBuffer::JoinSnapshot*>(snapshot);
        return s->ownFieldPtr(static_cast<std::size_t>(fieldIndex));
    }

    const void *rtJoinSnapshotGetCandidatePtr(void *snapshot, const std::uint64_t index, const std::uint64_t fieldIndex) {
        auto* s = static_cast<physical::details::JoinBuffer::JoinSnapshot*>(snapshot);
        return s->candidateFieldPtr(static_cast<std::size_t>(index), static_cast<std::size_t>(fieldIndex));
    }

    std::int64_t rtJoinSnapshotGetCandidateStartTimestamp(void *snapshot, const std::uint64_t index) {
        return static_cast<physical::details::JoinBuffer::JoinSnapshot*>(snapshot)->candidateStarts.at(index);
    }

    std::int64_t rtJoinSnapshotGetCandidateEndTimestamp(void *snapshot, const std::uint64_t index) {
        return static_cast<physical::details::JoinBuffer::JoinSnapshot*>(snapshot)->candidateEnds.at(index);
    }

    std::int64_t rtJoinSnapshotGetOwnStartTimestamp(void *snapshot) {
        return static_cast<physical::details::JoinBuffer::JoinSnapshot*>(snapshot)->startTs;
    }

    std::int64_t rtJoinSnapshotGetCandidateStartTibreaker(void* snapshot, const std::uint64_t index) {
        return static_cast<physical::details::JoinBuffer::JoinSnapshot*>(snapshot)->candidateStartsTb.at(index);
    }

    std::int64_t rtJoinSnapshotGetCandidateEndTibreaker(void* snapshot, const std::uint64_t index) {
        return static_cast<physical::details::JoinBuffer::JoinSnapshot*>(snapshot)->candidateEndsTb.at(index);
    }

    std::int64_t rtJoinSnapshotGetOwnStartTibreaker(void* snapshot) {
        return static_cast<physical::details::JoinBuffer::JoinSnapshot*>(snapshot)->startTb;
    }

    std::int64_t rtJoinSnapshotGetOwnEndTibreaker(void* snapshot) {
        return static_cast<physical::details::JoinBuffer::JoinSnapshot*>(snapshot)->endTb;
    }

    std::int64_t rtJoinSnapshotGetOwnEndTimestamp(void *snapshot) {
        return static_cast<physical::details::JoinBuffer::JoinSnapshot*>(snapshot)->endTs;
    }

    std::uint64_t rtJoinSnapshotGetCount(void *snapshot) {
        auto* s = static_cast<physical::details::JoinBuffer::JoinSnapshot*>(snapshot);
        return static_cast<std::uint64_t>(s->candidateCount);
    }

    void rtJoinSnapshotRelease(void *snapshot) {
        physical::details::JoinBuffer::releaseSnapshot(static_cast<physical::details::JoinBuffer::JoinSnapshot*>(snapshot));
    }

    bool rtJoinSnapshotGetWasLeft(void *snapshot) {
        return static_cast<physical::details::JoinBuffer::JoinSnapshot *>(snapshot)->wasLeft;
    }

    void *rtJoinWatermarkCreate(void * /*typeRegistryRawPtr*/) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        return new details::JoinWatermarkState();
    }

    void rtJoinWatermarkDestroy(void *state) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete static_cast<details::JoinWatermarkState *>(state);
    }

    void rtJoinMarkCandidateForRemove(void* state, void* snapshot, const std::uint64_t candidateIdx) {
        auto* joinBuffer = static_cast<physical::details::JoinBuffer*>(state);
        auto* snap = static_cast<physical::details::JoinBuffer::JoinSnapshot*>(snapshot);
        joinBuffer->markCandidateForRemove(snap, candidateIdx);
    }

    void rtJoinMarkCandidateAsMatch(void* state, void* snapshot, const std::uint64_t candidateIdx) {
        auto* joinBuffer = static_cast<physical::details::JoinBuffer*>(state);
        auto* snap = static_cast<physical::details::JoinBuffer::JoinSnapshot*>(snapshot);
        joinBuffer->markCandidateAsMatch(snap, candidateIdx);
    }

    void rtJoinRemoveMarkedCandidates(void* state, void* snapshot) {
        auto* joinBuffer = static_cast<physical::details::JoinBuffer*>(state);
        auto* snap = static_cast<physical::details::JoinBuffer::JoinSnapshot*>(snapshot);
        joinBuffer->removeMarkedCandidates(snap, snap->wasLeft);
    }

    std::uint64_t rtJoinSnapshotGetMatchCount(void* snapshot) {
        const auto* snap = static_cast<physical::details::JoinBuffer::JoinSnapshot*>(snapshot);
        return snap != nullptr ? snap->matchIndicies.size() : 0;
    }

    std::uint64_t rtJoinSnapshotGetMatchIndex(void* snapshot, const std::uint64_t matchIndex) {
        const auto* snap = static_cast<physical::details::JoinBuffer::JoinSnapshot*>(snapshot);
        if (snap == nullptr || matchIndex >= snap->matchIndicies.size()) {
            return 0;
        }
        return snap->matchIndicies.at(matchIndex);
    }

    // NOLINTNEXTLINE(readability-function-size)
    std::uint8_t rtJoinUpdateWatermark(void *state, const std::uint8_t isLeft, const std::int64_t inWatermark,
                                       const std::uint8_t inIsFinal, const std::int64_t allowedLateness,
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

    void rtJoinEvict(void* state, const std::int64_t threshold, const std::uint8_t callFromLeft, const std::uint8_t isFinal) {
        static_cast<physical::details::JoinBuffer*>(state)->evict(threshold, callFromLeft != 0, isFinal != 0);
    }

    void rtJoinMatchLeft(void* state, const std::int64_t startTs, const std::int64_t endTs, const std::int64_t startTb, const std::int64_t endTb) {
        static_cast<physical::details::JoinBuffer*>(state)->matchLeft(startTs, endTs, startTb, endTb);
    }

    void rtJoinMatchRight(void* state, const std::int64_t startTs, const std::int64_t endTs, const std::int64_t startTb, const std::int64_t endTb) {
        static_cast<physical::details::JoinBuffer*>(state)->matchRight(startTs, endTs, startTb, endTb);
    }
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::runtime