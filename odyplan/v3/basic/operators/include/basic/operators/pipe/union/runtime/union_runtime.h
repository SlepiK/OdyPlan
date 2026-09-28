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
    void *rtUnionWatermarkCreate(std::size_t numInputs);

    void rtUnionWatermarkDestroy(void *state);

    std::uint8_t rtUnionUpdateWatermark(void *state, std::size_t inputIndex, core::shared::types::Timestamp incomingWatermark,
                                        std::uint8_t incomingIsFinal, core::shared::types::Timestamp *outWatermark,
                                        std::uint8_t *outIsFinal);

    core::shared::types::Timestamp rtUnionGetGlobalWatermark(void* state);

    void *rtUnionBufferCreate(std::uint8_t outOfOrder,
        const core::shared::types::ValueTypeId* typeIds,
        std::size_t fieldSize);

    void rtUnionBufferDestroy(void *state);

    void rtUnionBufferBegin(void *state);

    void rtUnionBufferPush(void *state, std::size_t index, const void *data);

    void rtUnionBufferEnd(void *state, core::shared::identifiers::PortId inputPort,
        core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs,
        core::shared::types::TiBreak startTb, core::shared::types::TiBreak endTb);

    std::size_t rtUnionBufferGetCount(void *state);

    std::uint8_t rtUnionBufferIsEmpty(void *state);

    core::shared::types::Timestamp rtUnionBufferGetMinStartTimestamp(void *state);

    const std::byte *rtUnionBufferGetData(void *state, std::size_t index);

    const std::byte *rtUnionBufferGetFieldPtr(void *state,
        std::size_t rowIndex, std::size_t fieldIndex);

    core::shared::types::Timestamp rtUnionBufferGetStartTimestamp(void *state, std::size_t index);

    core::shared::types::Timestamp rtUnionBufferGetEndTimestamp(void *state, std::size_t index);

    core::shared::types::TiBreak rtUnionBufferGetStartTibreaker(void *state, std::size_t index);

    core::shared::types::TiBreak rtUnionBufferGetEndTibreaker(void *state, std::size_t index);

    core::shared::identifiers::PortId rtUnionBufferGetInputPort(void *state, std::size_t index);

    void rtUnionBufferRemoveV(void* state, const std::size_t* indices, std::size_t count);

    void rtUnionLock(void *state);

    void rtUnionUnlock(void *state);

    void rtUnionLockShared(void *state);

    void rtUnionUnlockShared(void *state);

    using BatchResult_T = struct {
        std::size_t count;
        std::size_t* indices;
    };

    BatchResult_T rtUnionBufferGetReadyIndices(void* state, core::shared::types::Timestamp watermark, std::size_t maxCount);

    std::size_t rtUnionBufferGetBatchCount(BatchResult_T result);

    std::size_t rtUnionBufferGetBatchElement(BatchResult_T result, std::size_t index);
    }
}

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_RUNTIME_UNION_RUNTIME_HPP