#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_RUNTIME_TIME_WINDOW_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_RUNTIME_TIME_WINDOW_RUNTIME_HPP
#include <algorithm>
#include <map>
#include <basic/window/count/count_window_buffer.h>
#include <basic/window/window_buffer.h>
#include <memory>
#include <vector>

namespace sierra::odyplan::v3::basic::operators::pipe::window::runtime {
    extern "C" {
        void* rtCreateTimeWindowBuffer(void* runtimeTypeRegistry, const core::shared::types::ValueTypeId* typeIds, std::uint64_t fieldCount);

        void rtDestroyTimeWindowBuffer(void* state);

        void rtTimeWindowBeginRow(void* state);

        void rtTimeWindowPush(void* state, std::uint64_t fieldIndex, const void* rawData);

        void rtTimeWindowEndRow(void* state, std::int64_t eventTs, std::int64_t windowStart, std::int64_t calculatedWindowEnd);

        void rtTimeWindowDrain(void* state, std::int64_t threshold, std::uint8_t isFinal);

        std::uint64_t rtTimeWindowGetDrainedRowCount(void* state);

        const void* rtTimeWindowGetDrainedData(void* state, std::uint64_t rowIndex, std::uint64_t fieldIndex);

        std::int64_t rtTimeWindowGetDrainedWindowStart(void* state, std::uint64_t rowIndex);

        std::int64_t rtTimeWindowGetDrainedWindowEnd(void* state, std::uint64_t rowIndex);

        std::int64_t rtTimeWindowGetDrainedTimestamp(void* state, std::uint64_t rowIndex);

        std::uint8_t rtTimeWindowGetDrainedIsLastInWindow(void* state, std::uint64_t rowIndex);
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::runtime

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_RUNTIME_TIME_WINDOW_RUNTIME_HPP
