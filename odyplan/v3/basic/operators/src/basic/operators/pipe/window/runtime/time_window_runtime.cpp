#include <basic/operators/pipe/window/runtime/time_window_runtime.h>

#include "basic/window/time/time_window_buffer.h"

namespace sierra::odyplan::v3::basic::operators::pipe::window::runtime {
    extern "C" {
    void *rtCreateTimeWindowBuffer(void *runtimeTypeRegistry, const core::shared::types::ValueTypeId *typeIds,
                                   const std::uint64_t fieldCount) {
        const auto* registry = static_cast<core::runtime::types::RuntimeTypeRegistry*>(runtimeTypeRegistry);
        core::logical::port::Port schema;

        for (std::uint64_t i = 0; i < fieldCount; ++i) {
            schema.push_back(core::logical::port::PortDescription{.valueTypeId = typeIds[i]});
        }

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        return new basic::window::time::TimeWindowBuffer(std::move(schema), *registry);
    }

    void rtDestroyTimeWindowBuffer(void *state) {
        auto* buffer = static_cast<basic::window::time::ITimeWindowBuffer*>(state);
        buffer->close();
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete buffer;
    }

    void rtTimeWindowBeginRow(void *state) {
        static_cast<basic::window::time::ITimeWindowBuffer*>(state)->begin();
    }

    void rtTimeWindowPush(void *state, const std::uint64_t fieldIndex, const void *rawData) {
        static_cast<basic::window::time::ITimeWindowBuffer*>(state)->push(fieldIndex, rawData);
    }

    void rtTimeWindowEndRow(void *state, const std::int64_t eventTs, const std::int64_t windowStart,
                            const std::int64_t calculatedWindowEnd) {
        static_cast<basic::window::time::ITimeWindowBuffer*>(state)->end(eventTs, windowStart, calculatedWindowEnd);
    }

    void rtTimeWindowDrain(void *state, const std::int64_t threshold, const std::uint8_t isFinal) {
        static_cast<basic::window::time::ITimeWindowBuffer*>(state)->drain(threshold, isFinal != 0);
    }

    std::uint64_t rtTimeWindowGetDrainedRowCount(void *state) {
        return static_cast<basic::window::time::ITimeWindowBuffer*>(state)->getDrainedRowCount();
    }

    const void *rtTimeWindowGetDrainedData(void *state, const std::uint64_t rowIndex, const std::uint64_t fieldIndex) {
        return static_cast<basic::window::time::ITimeWindowBuffer*>(state)->getDrainedData(rowIndex, fieldIndex);
    }

    std::int64_t rtTimeWindowGetDrainedWindowStart(void *state, const std::uint64_t rowIndex) {
        return static_cast<basic::window::time::ITimeWindowBuffer*>(state)->getDrainedWindowStart(rowIndex);
    }

    std::int64_t rtTimeWindowGetDrainedWindowEnd(void *state, const std::uint64_t rowIndex) {
        return static_cast<basic::window::time::ITimeWindowBuffer*>(state)->getDrainedWindowEnd(rowIndex);
    }

    std::int64_t rtTimeWindowGetDrainedTimestamp(void *state, const std::uint64_t rowIndex) {
        return static_cast<basic::window::time::ITimeWindowBuffer*>(state)->getDrainedTimestamp(rowIndex);
    }

    std::uint8_t rtTimeWindowGetDrainedIsLastInWindow(void* state, const std::uint64_t rowIndex) {
        return static_cast<basic::window::time::ITimeWindowBuffer*>(state)->getDrainedIsLastInWindow(rowIndex) ? 1 : 0;
    }
    }
}
