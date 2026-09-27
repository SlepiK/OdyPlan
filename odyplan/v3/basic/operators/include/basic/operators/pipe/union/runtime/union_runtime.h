#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_RUNTIME_UNION_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_RUNTIME_UNION_RUNTIME_HPP

#include <atomic>
#include <mutex>
#include <vector>
#include <basic/operators/pipe/union/runtime/union_buffer.h>
#include <core/shared/types/global_types.h>

namespace sierra::odyplan::v3::basic::operators::pipe::union_::runtime {// NOLINT(readability-identifier-naming)
    namespace details {
        struct UnionWatermarkState {
            std::atomic_flag lock = ATOMIC_FLAG_INIT;
            std::vector<core::shared::types::Timestamp> perInputWatermark;
            std::vector<bool> hasReported;
            std::vector<bool> perInputFinal;
            core::shared::types::Timestamp lastEmittedWatermark = std::numeric_limits<core::shared::types::Timestamp>::min();;
            bool finalEmitted = false;
        };
    }

    extern "C" {
    void *rtUnionWatermarkCreate(void* runtimeTypeRegistry, std::int64_t numInputs);

    void rtUnionWatermarkDestroy(void *state);

    std::uint8_t rtUnionUpdateWatermark(void *state, std::int64_t inputIndex, core::shared::types::Timestamp incomingWatermark,
                                        std::uint8_t incomingIsFinal, core::shared::types::Timestamp *outWatermark,
                                        std::uint8_t *outIsFinal);

    core::shared::types::Timestamp rtUnionGetGlobalWatermark(void* state);

    void *rtUnionBufferCreate(const core::runtime::types::RuntimeTypeRegistry *registry, std::uint8_t outOfOrder,
        const core::shared::types::ValueTypeId* typeIds,
        std::uint64_t fieldSize);

    void rtUnionBufferDestroy(void *state);

    void rtUnionBufferBegin(void *state);

    void rtUnionBufferPush(void *state, std::uint64_t index, const void *data);

    void rtUnionBufferEnd(void *state, std::uint64_t inputPort,
        core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs,
        core::shared::types::TiBreak startTb, core::shared::types::TiBreak endTb);

    std::uint64_t rtUnionBufferGetCount(void *state);

    std::uint8_t rtUnionBufferIsEmpty(void *state);

    core::shared::types::Timestamp rtUnionBufferGetMinStartTimestamp(void *state);

    const std::byte *rtUnionBufferGetData(void *state, std::uint64_t index);

    const std::byte *rtUnionBufferGetFieldPtr(void *state,
        std::uint64_t rowIndex, std::uint64_t fieldIndex);

    core::shared::types::Timestamp rtUnionBufferGetStartTimestamp(void *state, std::uint64_t index);

    core::shared::types::Timestamp rtUnionBufferGetEndTimestamp(void *state, std::uint64_t index);

    std::int64_t rtUnionBufferGetStartTibreaker(void *state, std::uint64_t index);

    std::int64_t rtUnionBufferGetEndTibreaker(void *state, std::uint64_t index);

    std::uint64_t rtUnionBufferGetInputPort(void *state, std::uint64_t index);

    void rtUnionBufferRemoveV(void* state, const std::uint64_t* indices, std::uint64_t count);

    void rtUnionLock(void *state);

    void rtUnionUnlock(void *state);

    void rtUnionLockShared(void *state);

    void rtUnionUnlockShared(void *state);

    using BatchResult_T = struct {
        std::uint64_t count;
        std::uint64_t* indices;
    };

    BatchResult_T rtUnionBufferGetReadyIndices(void* state, core::shared::types::Timestamp watermark, std::uint64_t maxCount);

    std::int64_t rtUnionBufferGetBatchCount(BatchResult_T result);

    std::int64_t rtUnionBufferGetBatchElement(BatchResult_T result, std::uint64_t index);
    }
}

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_RUNTIME_UNION_RUNTIME_HPP