#include <basic/operators/pipe/merge/runtime/merge_runtime.h>

namespace sierra::odyplan::v3::basic::operators::pipe::merge::runtime {
    extern "C" {
    void *rtMergeWatermarkCreate(void* /*runtimeTypeRegistry*/, std::uint64_t numInputs) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto* state = new details::MergeWatermarkState();
        state->perInputWatermark.assign(static_cast<std::size_t>(numInputs), 0);
        state->hasReported.assign(static_cast<std::size_t>(numInputs), false);
        state->perInputFinal.assign(static_cast<std::size_t>(numInputs), false);
        return state;
    }

    void rtMergeWatermarkDestroy(void *state) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete static_cast<details::MergeWatermarkState*>(state);
    }

    std::uint8_t rtMergeUpdateWatermark(void *state, std::uint64_t inputIndex, std::uint64_t incomingWatermark,
                                            std::uint8_t incomingIsFinal, std::uint64_t *outWatermark,
                                            std::uint8_t *outIsFinal) {
        auto* wState = static_cast<details::MergeWatermarkState*>(state);

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

        std::uint64_t effMin = std::numeric_limits<std::uint64_t>::max();
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