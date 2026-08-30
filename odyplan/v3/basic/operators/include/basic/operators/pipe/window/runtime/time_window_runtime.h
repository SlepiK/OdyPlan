#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_RUNTIME_TIME_WINDOW_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_RUNTIME_TIME_WINDOW_RUNTIME_HPP
#include <algorithm>
#include <map>
#include <basic/window/count/count_window_buffer.h>
#include <basic/window/window_buffer.h>
#include <memory>
#include <vector>

namespace sierra::odyplan::v3::basic::operators::pipe::window::runtime {
    namespace details {
        struct TimeWindowBucket {
            std::uint64_t windowStart{0};
            std::vector<void*> rowPtrs;
        };

        struct TimeWindowState {
            std::atomic_flag lock = ATOMIC_FLAG_INIT;
            std::map<std::uint64_t, TimeWindowBucket> currentOpenWindows;
        };
    }
    extern "C" {
        void* rtTimeWindowCreate(void* runtimeTypeRegistry);

        void rtTimeWindowDestroy(void* state);

        void rtTimeWindowInsert(void* state, std::uint64_t windowStart, std::uint64_t calculatedWindowEnd, void* rowPtr);

        void rtTimeWindowDrain(void* state, std::uint64_t threshold, std::uint8_t isFinal, void(*continuationFn)(void*));
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::runtime

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_RUNTIME_TIME_WINDOW_RUNTIME_HPP
