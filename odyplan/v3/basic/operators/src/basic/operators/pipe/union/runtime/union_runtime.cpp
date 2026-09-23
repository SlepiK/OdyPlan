#include <basic/operators/pipe/union/runtime/union_runtime.h>

namespace sierra::odyplan::v3::basic::operators::pipe::union_::runtime {
    extern "C" {
    void *rtUnionWatermarkCreate(void* /*runtimeTypeRegistry*/, const std::int64_t numInputs) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto* state = new details::UnionWatermarkState();
        const auto cNumInputs = static_cast<std::size_t>(numInputs);
        state->perInputWatermark.assign(cNumInputs, 0);
        state->hasReported.assign(cNumInputs, false);
        state->perInputFinal.assign(cNumInputs, false);
        return state;
    }

    void rtUnionWatermarkDestroy(void *state) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete static_cast<details::UnionWatermarkState*>(state);
    }

    std::uint8_t rtUnionUpdateWatermark(void *state, const std::int64_t inputIndex, const std::int64_t incomingWatermark,
                                            const std::uint8_t incomingIsFinal, std::int64_t *outWatermark,
                                            std::uint8_t *outIsFinal) {
        auto* wState = static_cast<details::UnionWatermarkState*>(state);

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
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::merge::runtime