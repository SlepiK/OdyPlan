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
                       std::uint64_t leftKeyFieldIndex,
                       std::uint64_t rightKeyFieldIndex,
                       core::shared::types::ValueTypeId keyTypeId);

    void rtJoinDestroy(void* state);

    void rtJoinLock(void* state);

    void rtJoinUnlock(void* state);

    void rtJoinBeginLeft(void* state);

    void rtJoinBeginRight(void* state);

    void rtJoinPushLeft(void* state, std::uint64_t index, const void* data);

    void rtJoinPushRight(void* state, std::uint64_t index, const void* data);

    void rtJoinEndLeft(void* state, std::int64_t timestamp);

    void rtJoinEndRight(void* state, std::int64_t timestamp);

    std::uint64_t rtJoinGetCandidateSize(void* state);

    const void*
    rtJoinGetCandidatePtr(void* state, std::uint64_t candidateIndex, std::uint64_t fieldIndex);

    std::int64_t rtJoinGetCandidateTimestamp(void* state, std::uint64_t candidateIndex);

    void rtJoinEvict(void* state, std::int64_t timestampMs, bool callFromLeft);

    std::uint64_t rtJoinSnapshotAndUnlock(void* state, std::uint64_t timestamp, bool callFromLeft);

    const void* rtJoinGetSnapshotPtr(void* state, std::uint64_t candidateIndex, std::uint64_t fieldIndex);

    std::uint64_t rtJoinGetSnapshotTimestamp(void* state, std::uint64_t candidateIndex);

    void rtJoinReleaseSnapshot(void* state);
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::runtime

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_RUNTIME_JOIN_RUNTIME_HPP