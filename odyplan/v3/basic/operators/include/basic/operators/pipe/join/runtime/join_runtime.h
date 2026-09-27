#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_RUNTIME_JOIN_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_RUNTIME_JOIN_RUNTIME_HPP

#include <basic/operators/pipe/join/runtime/join_buffer.h>
#include <core/logical/port/port.h>
#include <cstdint>

namespace sierra::odyplan::v3::basic::operators::pipe::join::runtime {
    extern "C" {
    // NOLINTNEXTLINE(readability-function-size)
    void* rtJoinCreate(const void* typeRegistryRawPtr,
                       const core::shared::types::ValueTypeId* leftTypeIds,
                       std::uint64_t leftFieldSize,
                       const core::shared::types::ValueTypeId* rightTypeIds,
                       std::uint64_t rightFieldSize,
                       std::uint8_t hasEquiKey,
                       const std::uint64_t* leftKeyFieldIndices,
                       const std::uint64_t* rightKeyFieldIndices,
                       const core::shared::types::ValueTypeId* keyTypeIds,
                       std::uint64_t equiKeyCnt,
                       uint8_t outOfOrder);

    void rtJoinDestroy(void* state);

    void rtJoinLock(void* state);

    void rtJoinUnlock(void* state);

    void rtJoinBeginLeft(void* state);

    void rtJoinBeginRight(void* state);

    void rtJoinPushLeft(void* state, std::uint64_t index, const void* data);

    void rtJoinPushRight(void* state, std::uint64_t index, const void* data);

    void rtJoinEndLeft(void* state, core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs, core::shared::types::TiBreak startTb, core::shared::types::TiBreak endTb);

    void rtJoinEndRight(void* state, core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs, core::shared::types::TiBreak startTb, core::shared::types::TiBreak endTb);

    void* rtJoinSnapshotForCompute(void* state, uint8_t callFromLeft, uint8_t extract);

    const void* rtJoinSnapshotGetOwnPtr(void* snapshot, std::uint64_t fieldIndex);

    const void* rtJoinSnapshotGetCandidatePtr(void* snapshot, std::uint64_t index, std::uint64_t fieldIndex);

    core::shared::types::Timestamp rtJoinSnapshotGetCandidateStartTimestamp(void* snapshot, std::uint64_t index);

    core::shared::types::Timestamp rtJoinSnapshotGetCandidateEndTimestamp(void* snapshot, std::uint64_t index);

    core::shared::types::TiBreak rtJoinSnapshotGetCandidateStartTibreaker(void* snapshot, std::uint64_t index);

    core::shared::types::TiBreak rtJoinSnapshotGetCandidateEndTibreaker(void* snapshot, std::uint64_t index);

    core::shared::types::Timestamp rtJoinSnapshotGetOwnStartTimestamp(void* snapshot);

    core::shared::types::Timestamp rtJoinSnapshotGetOwnEndTimestamp(void* snapshot);

    core::shared::types::TiBreak rtJoinSnapshotGetOwnStartTibreaker(void* snapshot);

    core::shared::types::TiBreak rtJoinSnapshotGetOwnEndTibreaker(void* snapshot);

    std::uint64_t rtJoinSnapshotGetCount(void* snapshot);

    void rtJoinSnapshotRelease(void *snapshot);

    bool rtJoinSnapshotGetWasLeft(void *snapshot);

    void *rtJoinWatermarkCreate(void *typeRegistryRawPtr);

    void rtJoinWatermarkDestroy(void *state);

    void rtJoinMarkCandidateForRemove(void* state, void* snapshot, std::uint64_t candidateIdx);

    void rtJoinMarkCandidateAsMatch(void* state, void* snapshot, std::uint64_t candidateIdx);

    void rtJoinRemoveMarkedCandidates(void* state, void* snapshot);

    void rtJoinMarkAllCandidatesAsMatch(void* state, void* snapshot, std::uint8_t extract);

    std::uint64_t rtJoinSnapshotGetMatchCount(void* snapshot);

    std::uint64_t rtJoinSnapshotGetMatchIndex(void* snapshot, std::uint64_t matchIndex);

    std::uint8_t rtJoinUpdateWatermark(void *state, std::uint8_t isLeft, core::shared::types::Timestamp inWatermark,
                                       std::uint8_t inIsFinal, core::shared::types::Timestamp allowedLateness,
                                       std::int64_t *outEvictLeftTh, std::int64_t *outEvictRightTh,
                                       std::int64_t *outGlobalWatermark, std::uint8_t *outIsFinal);

    void rtJoinEvict(void* state, core::shared::types::Timestamp threshold, std::uint8_t callFromLeft, std::uint8_t isFinal);

    void rtJoinMatchLeft(void* state, core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs, core::shared::types::TiBreak startTb, core::shared::types::TiBreak endTb);

    void rtJoinMatchRight(void* state, core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs, core::shared::types::TiBreak startTb, core::shared::types::TiBreak endTb);

    void rtJoinBufferOutput(void* state, const void* ownRowPtr, std::uint64_t ownRowSize, const void* candRowPtr, std::uint64_t candRowSize, std::uint8_t onLeft, core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs, core::shared::types::TiBreak startTb, core::shared::types::TiBreak endTb);

    std::uint64_t rtJoinDrainOutput(void* state, core::shared::types::Timestamp watermark, std::uint64_t maxCount, std::uint64_t* outIndices);

    const void* rtJoinGetOutputRowPtr(void* state, std::uint64_t index);

    core::shared::types::Timestamp rtJoinGetOutputRowStartTs(void* state, std::uint64_t index);

    core::shared::types::Timestamp rtJoinGetOutputRowEndTs(void* state, std::uint64_t index);

    core::shared::types::TiBreak rtJoinGetOutputRowStartTb(void* state, std::uint64_t index);

    core::shared::types::TiBreak rtJoinGetOutputRowEndTb(void* state, std::uint64_t index);

    std::uint64_t rtJoinGetOutputRowSize(void* state);

    const void* rtJoinSnapshotGetOwnRowPtr(void* snapshot);

    std::uint64_t rtJoinSnapshotGetOwnRowSize(void* snapshot);

    const void* rtJoinSnapshotGetCandidateRowPtr(void* snapshot, std::uint64_t index);

    std::uint64_t rtJoinSnapshotGetCandidateRowSize(void* snapshot);

    void rtJoinRemoveOutputRows(void* state, const std::uint64_t* indices, std::uint64_t count);

    core::shared::types::Timestamp rtJoinGetMinStartTimestampLeft(void* state);

    core::shared::types::Timestamp rtJoinGetMinStartTimestampRight(void* state);

    core::shared::types::Timestamp rtJoinGetGlobalWatermark(void* state);
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::runtime

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_RUNTIME_JOIN_RUNTIME_HPP