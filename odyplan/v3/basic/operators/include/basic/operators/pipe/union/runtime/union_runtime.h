#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_RUNTIME_UNION_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_RUNTIME_UNION_RUNTIME_HPP

#include <atomic>
#include <mutex>
#include <vector>
#include <basic/operators/pipe/union/runtime/union_buffer.h>

namespace sierra::odyplan::v3::basic::operators::pipe::union_::runtime {// NOLINT(readability-identifier-naming)
    namespace details {
        struct UnionWatermarkState {
            std::atomic_flag lock = ATOMIC_FLAG_INIT;
            std::vector<std::int64_t> perInputWatermark;
            std::vector<bool> hasReported;
            std::vector<bool> perInputFinal;
            std::int64_t lastEmittedWatermark = std::numeric_limits<std::int64_t>::min();;
            bool finalEmitted = false;
        };
    }

    extern "C" {
    void *rtUnionWatermarkCreate(void* runtimeTypeRegistry, std::int64_t numInputs);

    void rtUnionWatermarkDestroy(void *state);

    std::uint8_t rtUnionUpdateWatermark(void *state, std::int64_t inputIndex, std::int64_t incomingWatermark,
                                        std::uint8_t incomingIsFinal, std::int64_t *outWatermark,
                                        std::uint8_t *outIsFinal);

    std::int64_t rtUnionGetGlobalWatermark(void* state);

    void *rtUnionBufferCreate(const core::runtime::types::RuntimeTypeRegistry *registry, std::uint8_t outOfOrder,
        const core::shared::types::ValueTypeId* typeIds,
        std::uint64_t fieldSize);

    void rtUnionBufferDestroy(void *state);

    void rtUnionBufferBegin(void *state);

    void rtUnionBufferPush(void *state, std::uint64_t index, const void *data);

    void rtUnionBufferEnd(void *state, std::uint64_t inputPort,
        std::int64_t startTs, std::int64_t endTs,
        std::int64_t startTb, std::int64_t endTb);

    std::uint64_t rtUnionBufferGetCount(void *state);

    std::uint8_t rtUnionBufferIsEmpty(void *state);

    std::int64_t rtUnionBufferGetMinStartTimestamp(void *state);

    const std::byte *rtUnionBufferGetData(void *state, std::uint64_t index);

    const std::byte *rtUnionBufferGetFieldPtr(void *state,
        std::uint64_t rowIndex, std::uint64_t fieldIndex);

    std::int64_t rtUnionBufferGetStartTimestamp(void *state, std::uint64_t index);

    std::int64_t rtUnionBufferGetEndTimestamp(void *state, std::uint64_t index);

    std::int64_t rtUnionBufferGetStartTibreaker(void *state, std::uint64_t index);

    std::int64_t rtUnionBufferGetEndTibreaker(void *state, std::uint64_t index);

    std::uint64_t rtUnionBufferGetInputPort(void *state, std::uint64_t index);

    void rtUnionBufferRemoveV(void* state, const std::uint64_t* indices, std::uint64_t count);

    void rtUnionLock(void* state);

    void rtUnionUnlock(void* state);

    std::uint64_t rtUnionBufferGetMinStartTimestampIndex(void* state, std::int64_t watermark);
    }
}

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_RUNTIME_UNION_RUNTIME_HPP