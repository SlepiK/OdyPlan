#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MERGE_RUNTIME_MERGE_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MERGE_RUNTIME_MERGE_RUNTIME_HPP

#include <atomic>
#include <mutex>
#include <vector>

namespace sierra::odyplan::v3::basic::operators::pipe::merge::runtime {
    namespace details {
        struct MergeWatermarkState {
            std::atomic_flag lock = ATOMIC_FLAG_INIT;
            std::vector<std::uint64_t> perInputWatermark;
            std::vector<bool> hasReported;
            std::vector<bool> perInputFinal;
            std::uint64_t lastEmittedWatermark = 0;
            bool finalEmitted = false;
        };
    }

    extern "C" {
    void *rtMergeWatermarkCreate(void* runtimeTypeRegistry, std::uint64_t numInputs);

    void rtMergeWatermarkDestroy(void *state);

    std::uint8_t rtMergeUpdateWatermark(void *state, std::uint64_t inputIndex, std::uint64_t incomingWatermark,
                                        std::uint8_t incomingIsFinal, std::uint64_t *outWatermark,
                                        std::uint8_t *outIsFinal);
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::merge::runtime

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MERGE_RUNTIME_MERGE_RUNTIME_HPP