#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_RUNTIME_UNION_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_RUNTIME_UNION_RUNTIME_HPP

#include <atomic>
#include <mutex>
#include <vector>

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
    }
}

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_RUNTIME_UNION_RUNTIME_HPP