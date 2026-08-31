#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_RUNTIME_JOIN_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_RUNTIME_JOIN_RUNTIME_HPP

#include <basic/operators/pipe/join/physical/details/join_buffer.h>
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
                       std::uint64_t leftWindowSize,
                       std::uint64_t rightWindowSize,
                       std::uint8_t hasEquiKey,
                       const std::uint64_t* leftKeyFieldIndices,
                       const std::uint64_t* rightKeyFieldIndices,
                       const core::shared::types::ValueTypeId* keyTypeIds,
                       std::uint64_t equiKeyCnt);

    void rtJoinDestroy(void* state);

    void rtJoinLock(void* state);

    void rtJoinUnlock(void* state);

    void rtJoinBeginLeft(void* state);

    void rtJoinBeginRight(void* state);

    void rtJoinPushLeft(void* state, std::uint64_t index, const void* data);

    void rtJoinPushRight(void* state, std::uint64_t index, const void* data);

    void rtJoinEndLeft(void* state, std::int64_t timestamp);

    void rtJoinEndRight(void* state, std::int64_t timestamp);

    void* rtJoinSnapshotForComputeAndUnlock(void* state, uint8_t callFromLeft);

    const void* rtJoinSnapshotGetOwnPtr(void* snapshot, std::uint64_t fieldIndex);

    const void* rtJoinSnapshotGetCandidatePtr(void* snapshot, std::uint64_t index, std::uint64_t fieldIndex);

    std::uint64_t rtJoinSnapshotGetCandidateTimestamp(void* snapshot, std::uint64_t index);

    std::uint64_t rtJoinSnapshotGetOwnTimestamp(void* snapshot);

    std::uint64_t rtJoinSnapshotGetCount(void* snapshot);

    void rtJoinSnapshotRelease(void *snapshot);

    bool rtJoinSnapshotGetWasLeft(void *snapshot);

    void *rtJoinWatermarkCreate(void *typeRegistryRawPtr);

    void rtJoinWatermarkDestroy(void *state);

    std::uint8_t rtJoinUpdateWatermark(void *state, std::uint8_t isLeft, std::int64_t inWatermark,
                                       std::uint8_t inIsFinal, std::int64_t allowedLateness,
                                       std::int64_t *outEvictLeftTh, std::int64_t *outEvictRightTh,
                                       std::int64_t *outGlobalWatermark, std::uint8_t *outIsFinal);

    void rtJoinEvict(void* state, std::int64_t timestamp, std::uint8_t callFromLeft);
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::runtime

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_RUNTIME_JOIN_RUNTIME_HPP