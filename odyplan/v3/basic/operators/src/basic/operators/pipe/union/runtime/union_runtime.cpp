#include <basic/operators/pipe/union/runtime/union_runtime.h>

namespace sierra::odyplan::v3::basic::operators::pipe::union_::runtime {
    extern "C" {
    void *rtUnionWatermarkCreate(void * /*runtimeTypeRegistry*/, const std::int64_t numInputs) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto *state = new details::UnionWatermarkState();
        const auto cNumInputs = static_cast<std::size_t>(numInputs);
        state->perInputWatermark.assign(cNumInputs, 0);
        state->hasReported.assign(cNumInputs, false);
        state->perInputFinal.assign(cNumInputs, false);
        return state;
    }

    void rtUnionWatermarkDestroy(void *state) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete static_cast<details::UnionWatermarkState *>(state);
    }

    std::uint8_t rtUnionUpdateWatermark(void *state, const std::int64_t inputIndex,
                                        const std::int64_t incomingWatermark,
                                        const std::uint8_t incomingIsFinal, std::int64_t *outWatermark,
                                        std::uint8_t *outIsFinal) {

        auto *wState = static_cast<details::UnionWatermarkState *>(state);

        while (wState->lock.test_and_set(std::memory_order_acquire)) {
        }

        const bool isFinalBool = (incomingIsFinal != 0);

        const auto index = static_cast<std::size_t>(inputIndex);

        if (wState->hasReported[index]) {
            wState->perInputWatermark[index] = std::max(wState->perInputWatermark[index], incomingWatermark);
        } else {
            wState->perInputWatermark[index] = incomingWatermark;
            wState->hasReported[index] = true;
        }

        if (isFinalBool) {
            wState->perInputFinal[index] = true;
        }

        std::int64_t effMin = std::numeric_limits<std::int64_t>::max();
        bool allFinal = true;

        for (std::size_t i = 0; i < wState->perInputWatermark.size(); ++i) {
            if (wState->perInputFinal[i]) {
                continue;
            }
            allFinal = false;
            effMin = wState->hasReported[i] ? std::min(effMin, wState->perInputWatermark[i]) : 0;
        }

        std::uint8_t sEmit = 0;

        if (allFinal) {
            if (!wState->finalEmitted) {
                wState->finalEmitted = true;
                *outWatermark = std::numeric_limits<std::uint64_t>::max();
                *outIsFinal = 1;
                sEmit = 1;
            }
        } else if (effMin > wState->lastEmittedWatermark) {
            wState->lastEmittedWatermark = effMin;
            *outWatermark = effMin;
            *outIsFinal = 0;
            sEmit = 1;
        }

        wState->lock.clear(std::memory_order_release);

        return sEmit;
    }


    core::shared::types::Timestamp rtUnionGetGlobalWatermark(void *state) {
        auto *wState = static_cast<details::UnionWatermarkState *>(state);

        while (wState->lock.test_and_set(std::memory_order_acquire)) {
        }

        const std::int64_t watermark = wState->lastEmittedWatermark;

        wState->lock.clear(std::memory_order_release);

        return watermark;
    }

    void *rtUnionBufferCreate(const core::runtime::types::RuntimeTypeRegistry *registry, const std::uint8_t outOfOrder,
                              const core::shared::types::ValueTypeId *typeIds,
                              const std::uint64_t fieldSize) {
        core::logical::port::Port schema;

        for (std::uint64_t i = 0; i < fieldSize; ++i) {
            schema.push_back(
                core::logical::port::PortDescription{.valueTypeId = typeIds[i]});
        }

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        return new UnionBuffer(outOfOrder != 0, std::move(schema), registry);
    }

    void rtUnionBufferDestroy(void *state) {
        auto *buffer = static_cast<UnionBuffer *>(state);
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete buffer;
    }

    void rtUnionBufferBegin(void *state) {
        static_cast<UnionBuffer *>(state)->begin();
    }

    void rtUnionBufferPush(void *state, const std::uint64_t index, const void *data) {
        static_cast<UnionBuffer *>(state)->push(index, data);
    }

    void rtUnionBufferEnd(void *state, const std::uint64_t inputPort,
                          const std::int64_t startTs, const std::int64_t endTs,
                          const std::int64_t startTb, const std::int64_t endTb) {
        static_cast<UnionBuffer *>(state)->end(inputPort, startTs, endTs, startTb, endTb);
    }

    std::uint64_t rtUnionBufferGetCount(void *state) {
        return static_cast<UnionBuffer *>(state)->getCount();
    }

    std::uint8_t rtUnionBufferIsEmpty(void *state) {
        return static_cast<UnionBuffer *>(state)->isEmpty() ? 1 : 0;
    }

    core::shared::types::Timestamp rtUnionBufferGetMinStartTimestamp(void *state) {
        return static_cast<UnionBuffer *>(state)->getMinStartTimestamp();
    }

    const std::byte *rtUnionBufferGetData(void *state, const std::uint64_t index) {
        return static_cast<UnionBuffer *>(state)->getData(index);
    }

    const std::byte *rtUnionBufferGetFieldPtr(void *state,
                                              const std::uint64_t rowIndex, const std::uint64_t fieldIndex) {
        return static_cast<UnionBuffer *>(state)->getFieldPtr(rowIndex, fieldIndex);
    }

    core::shared::types::Timestamp rtUnionBufferGetStartTimestamp(void *state, const std::uint64_t index) {
        return static_cast<UnionBuffer *>(state)->getStartTimestamp(index);
    }

    core::shared::types::Timestamp rtUnionBufferGetEndTimestamp(void *state, const std::uint64_t index) {
        return static_cast<UnionBuffer *>(state)->getEndTimestamp(index);
    }

    std::int64_t rtUnionBufferGetStartTibreaker(void *state, const std::uint64_t index) {
        return static_cast<UnionBuffer *>(state)->getStartTibreaker(index);
    }

    std::int64_t rtUnionBufferGetEndTibreaker(void *state, const std::uint64_t index) {
        return static_cast<UnionBuffer *>(state)->getEndTibreaker(index);
    }

    std::uint64_t rtUnionBufferGetInputPort(void *state, const std::uint64_t index) {
        return static_cast<UnionBuffer *>(state)->getInputPort(index);
    }

    void rtUnionLock(void* state) {
        static_cast<UnionBuffer *>(state)->lock();
    }

    void rtUnionUnlock(void* state) {
        static_cast<UnionBuffer *>(state)->unlock();
    }

    void rtUnionLockShared(void *state) {
        static_cast<UnionBuffer *>(state)->lockShared();
    }

    void rtUnionUnlockShared(void *state) {
        static_cast<UnionBuffer *>(state)->unlockShared();
    }

    void rtUnionBufferRemoveV(void *state, const std::uint64_t *indices, const std::uint64_t count) {
        auto *buffer = static_cast<union_::runtime::UnionBuffer *>(state);

        buffer->removeV(static_cast<const void*>(indices), static_cast<std::int64_t>(count));
    }

    std::uint64_t rtUnionBufferGetMinStartTimestampIndex(void* state, const std::int64_t watermark) {
        auto *buffer = static_cast<union_::runtime::UnionBuffer*>(state);
        auto index = buffer->getMinStartTimestampIndex(static_cast<Timestamp>(watermark));
        return index.has_value() ? index.value() : -1;
    }

    BatchResult_T rtUnionBufferGetReadyIndices(void* state, const std::int64_t watermark, const std::uint64_t maxCount) {
        auto* buffer = static_cast<UnionBuffer*>(state);

        const auto result = buffer->getReadyIndices(static_cast<Timestamp>(watermark), static_cast<std::size_t>(maxCount));

        return BatchResult_T{
            .count = result.count,
            .indices = result.indices
        };
    }

    std::int64_t rtUnionBufferGetBatchCount(const BatchResult_T result) {
        return static_cast<std::int64_t>(result.count);
    }

    std::int64_t rtUnionBufferGetBatchElement(const BatchResult_T result, const std::uint64_t index) {
        if (index >= result.count) {
            return -1;
        }
        return static_cast<std::int64_t>(result.indices[index]);
    }
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::merge::runtime