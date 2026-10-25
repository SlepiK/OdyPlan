#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_RUNTIME_COUNT_WINDOW_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_RUNTIME_COUNT_WINDOW_RUNTIME_HPP
#include <algorithm>
#include <basic/operators/pipe/window/runtime/count_window_buffer.h>
#include <memory>
#include <vector>
#include <core/shared/types/global_types.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::runtime {
    extern "C" {
    void*
    rtCreateCountWindowBuffer(std::size_t size,
                              std::size_t slide,
                              std::size_t advance,
                              const core::shared::types::ValueTypeId* typeIds,
                              std::size_t fieldCount);

    void rtDestroyCountWindowBuffer(void* state);

    void rtCountWindowBeginRow(void* state);

    void rtCountWindowPush(void* state, std::size_t fieldIndex, const void* rawData);

    void rtCountWindowEndRow(void* state, core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs);

    bool rtCountWindowReady(void* state);

    std::size_t rtCountWindowRowCount(void* state);

    const void* rtCountWindowGetData(void* state, std::size_t rowIndex, std::size_t fieldIndex);

    core::shared::types::Timestamp rtCountWindowRowStart(void* state, std::size_t rowIndex);

    core::shared::types::Timestamp rtCountWindowEnd(void* state);

    void rtCountWindowAdvance(void* state);

    core::shared::types::Timestamp rtCountWindowGetMinTs(void* state);

    std::size_t rtCountWindowEmitCount(void* state);
    }
}

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_RUNTIME_COUNT_WINDOW_RUNTIME_HPP
