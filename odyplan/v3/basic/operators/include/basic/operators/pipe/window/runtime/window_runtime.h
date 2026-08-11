#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_RUNTIME_WINDOW_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_RUNTIME_WINDOW_RUNTIME_HPP
#include <algorithm>
#include <basic/window/count/count_window_buffer.h>
#include <basic/window/time/time_window_buffer.h>
#include <basic/window/window_buffer.h>
#include <memory>
#include <vector>

namespace sierra::odyplan::v3::basic::operators::pipe::window::runtime {
    extern "C" {
    void*
    rtCreateCountWindowBuffer(const void* typeRegistryRaw,
                              std::int64_t size,
                              std::int64_t slide,
                              const sierra::odyplan::v3::core::shared::types::ValueTypeId* typeIds,
                              std::uint64_t fieldCount);

    void rtDestroyCountWindowBuffer(void* state);

    void* rtCreateTimeWindowBuffer(const void* typeRegistryRaw,
                                   std::int64_t size,
                                   std::int64_t slide,
                                   std::int64_t allowedLateness,
                                   const std::uint8_t* typeIds,
                                   std::uint64_t fieldCount);

    void rtDestroyTimeWindowBuffer(void* state);

    void rtWindowBeginRow(void* state);

    void rtWindowPush(void* state, std::uint64_t fieldIndex, const void* rawData);

    void rtWindowEndRow(void* state, std::int64_t timestamp);

    void rtWindowCompute(void* state);

    void rtWindowFlush(void* state);

    std::uint64_t rtWindowGetCount(void* state);

    std::uint64_t rtWindowGetRowCountInWindow(void* state, std::uint64_t windowIndex);

    const void* rtWindowGetData(void* state,
                                uint64_t windowIndex,
                                uint64_t rowIndex,
                                uint64_t fieldIndex);

    std::int64_t rtWindowGetTimestamp(void* state,
                                      std::uint64_t windowIndex,
                                      std::uint64_t rowIndex);
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::runtime

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_RUNTIME_WINDOW_RUNTIME_HPP
