#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_RUNTIME_WINDOW_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_RUNTIME_WINDOW_RUNTIME_HPP
#include <algorithm>
#include <basic/window/count/count_window_buffer.h>
#include <memory>
#include <vector>

namespace sierra::odyplan::v3::basic::operators::pipe::window::runtime {
    extern "C" {
    void*
    rtCreateCountWindowBuffer(const void* typeRegistryRaw,
                              std::int64_t size,
                              std::int64_t slide,
                              std::int64_t advance,
                              const core::shared::types::ValueTypeId* typeIds,
                              std::uint64_t fieldCount);

    void rtDestroyCountWindowBuffer(void* state);

    void rtCountWindowBeginRow(void* state);

    void rtCountWindowPush(void* state, std::uint64_t fieldIndex, const void* rawData);

    void rtCountWindowEndRow(void* state, std::int64_t startTs, std::int64_t endTs);

    bool rtCountWindowReady(void* state);

    std::uint64_t rtCountWindowRowCount(void* state);

    const void* rtCountWindowGetData(void* state, std::uint64_t rowIndex, std::uint64_t fieldIndex);

    std::int64_t rtCountWindowRowStart(void* state, std::uint64_t rowIndex);

    std::int64_t rtCountWindowEnd(void* state);

    void rtCountWindowAdvance(void* state);

    void rtCountWindowFlush(void* state);
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::runtime

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_RUNTIME_WINDOW_RUNTIME_HPP
