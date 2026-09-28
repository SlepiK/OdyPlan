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
                       std::size_t leftFieldSize,
                       const core::shared::types::ValueTypeId* rightTypeIds,
                       std::size_t rightFieldSize,
                       std::uint8_t hasEquiKey,
                       const std::size_t* leftKeyFieldIndices,
                       const std::size_t* rightKeyFieldIndices,
                       const core::shared::types::ValueTypeId* keyTypeIds,
                       std::size_t equiKeyCnt,
                       uint8_t outOfOrder);

    void rtJoinDestroy(void* state);

    void rtJoinLock(void* state);

    void rtJoinUnlock(void* state);

    void rtJoinBeginLeft(void* state);

    void rtJoinBeginRight(void* state);

    void rtJoinPushLeft(void* state, std::size_t index, const void* data);

    void rtJoinPushRight(void* state, std::size_t index, const void* data);

    void rtJoinEndLeft(void* state, core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs, core::shared::types::TiBreak startTb, core::shared::types::TiBreak endTb);

    void rtJoinEndRight(void* state, core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs, core::shared::types::TiBreak startTb, core::shared::types::TiBreak endTb);

    void* rtJoinSnapshotForCompute(void* state, uint8_t callFromLeft, uint8_t extract);

    const void* rtJoinSnapshotGetOwnPtr(void* snapshot, std::size_t fieldIndex);

    const void* rtJoinSnapshotGetCandidatePtr(void* snapshot, std::size_t index, std::size_t fieldIndex);

    core::shared::types::Timestamp rtJoinSnapshotGetCandidateStartTimestamp(void* snapshot, std::size_t index);

    core::shared::types::Timestamp rtJoinSnapshotGetCandidateEndTimestamp(void* snapshot, std::size_t index);

    core::shared::types::TiBreak rtJoinSnapshotGetCandidateStartTibreaker(void* snapshot, std::size_t index);

    core::shared::types::TiBreak rtJoinSnapshotGetCandidateEndTibreaker(void* snapshot, std::size_t index);

    core::shared::types::Timestamp rtJoinSnapshotGetOwnStartTimestamp(void* snapshot);

    core::shared::types::Timestamp rtJoinSnapshotGetOwnEndTimestamp(void* snapshot);

    core::shared::types::TiBreak rtJoinSnapshotGetOwnStartTibreaker(void* snapshot);

    core::shared::types::TiBreak rtJoinSnapshotGetOwnEndTibreaker(void* snapshot);

    std::size_t rtJoinSnapshotGetCount(void* snapshot);

    void rtJoinSnapshotRelease(void *snapshot);

    bool rtJoinSnapshotGetWasLeft(void *snapshot);

    void *rtJoinWatermarkCreate(void *typeRegistryRawPtr);

    void rtJoinWatermarkDestroy(void *state);

    void rtJoinMarkCandidateForRemove(void* state, void* snapshot, std::size_t candidateIdx);

    void rtJoinMarkCandidateAsMatch(void* state, void* snapshot, std::size_t candidateIdx);

    void rtJoinRemoveMarkedCandidates(void* state, void* snapshot);

    void rtJoinMarkAllCandidatesAsMatch(void* state, void* snapshot, std::uint8_t extract);

    std::size_t rtJoinSnapshotGetMatchCount(void* snapshot);

    std::size_t rtJoinSnapshotGetMatchIndex(void* snapshot, std::size_t matchIndex);

    std::uint8_t rtJoinUpdateWatermark(void *state, std::uint8_t isLeft, core::shared::types::Timestamp inWatermark,
                                       std::uint8_t inIsFinal, core::shared::types::Timestamp allowedLateness,
                                       core::shared::types::Timestamp *outEvictLeftTh, core::shared::types::Timestamp *outEvictRightTh,
                                       core::shared::types::Timestamp *outGlobalWatermark, std::uint8_t *outIsFinal);

    void rtJoinEvict(void* state, core::shared::types::Timestamp threshold, std::uint8_t callFromLeft, std::uint8_t isFinal);

    void rtJoinMatchLeft(void* state, core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs, core::shared::types::TiBreak startTb, core::shared::types::TiBreak endTb);

    void rtJoinMatchRight(void* state, core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs, core::shared::types::TiBreak startTb, core::shared::types::TiBreak endTb);

    void rtJoinBufferOutput(void* state, const void* ownRowPtr, std::size_t ownRowSize, const void* candRowPtr, std::size_t candRowSize, std::uint8_t onLeft, core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs, core::shared::types::TiBreak startTb, core::shared::types::TiBreak endTb);

    std::size_t rtJoinDrainOutput(void* state, core::shared::types::Timestamp watermark, std::size_t maxCount, std::size_t* outIndices);

    const void* rtJoinGetOutputRowPtr(void* state, std::size_t index);

    core::shared::types::Timestamp rtJoinGetOutputRowStartTs(void* state, std::size_t index);

    core::shared::types::Timestamp rtJoinGetOutputRowEndTs(void* state, std::size_t index);

    core::shared::types::TiBreak rtJoinGetOutputRowStartTb(void* state, std::size_t index);

    core::shared::types::TiBreak rtJoinGetOutputRowEndTb(void* state, std::size_t index);

    std::size_t rtJoinGetOutputRowSize(void* state);

    const void* rtJoinSnapshotGetOwnRowPtr(void* snapshot);

    std::size_t rtJoinSnapshotGetOwnRowSize(void* snapshot);

    const void* rtJoinSnapshotGetCandidateRowPtr(void* snapshot, std::size_t index);

    std::size_t rtJoinSnapshotGetCandidateRowSize(void* snapshot);

    void rtJoinRemoveOutputRows(void* state, const std::size_t* indices, std::size_t count);

    core::shared::types::Timestamp rtJoinGetMinStartTimestampLeft(void* state);

    core::shared::types::Timestamp rtJoinGetMinStartTimestampRight(void* state);

    core::shared::types::Timestamp rtJoinGetGlobalWatermark(void* state);
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::runtime

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_RUNTIME_JOIN_RUNTIME_HPP