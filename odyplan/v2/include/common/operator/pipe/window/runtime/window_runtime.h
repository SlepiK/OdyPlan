#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_OPERATOR_RUNTIME_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_OPERATOR_RUNTIME_H
#include <algorithm>
#include <memory>
#include <vector>

#include "common/window/window_buffer.h"
#include "common/window/count/count_window_buffer.h"
#include "common/window/time/time_window_buffer.h"

inline std::vector<std::unique_ptr<sierra::odyplan::v2::common::window::IWindowBuffer>> gWindows;

inline std::uint64_t registerWindow(std::unique_ptr<sierra::odyplan::v2::common::window::IWindowBuffer> window) {
    gWindows.push_back(std::move(window));
    return gWindows.size() - 1;
}

inline void shutdownWindows() {
    for (const auto &window : gWindows) {
        window->close();
    }
    gWindows.clear();
}

struct WindowShutdownGuard {
    WindowShutdownGuard() = default;
    WindowShutdownGuard(WindowShutdownGuard const&) = delete;
    WindowShutdownGuard& operator =(WindowShutdownGuard const&) = delete;
    WindowShutdownGuard(WindowShutdownGuard&&) = delete;
    WindowShutdownGuard& operator=(WindowShutdownGuard&&) = delete;

    ~WindowShutdownGuard() {
        shutdownWindows();
    }
};

extern "C" {
    inline void* rtCreateCountWindowBuffer(const void* typeRegistryRaw, const std::int64_t size, const std::int64_t slide,
                                     const sierra::odyplan::v2::core::common::identifier::TypeId* typeIds, const std::uint64_t fieldCount) {
        const auto* typeRegistry = static_cast<const sierra::odyplan::v2::core::common::type::TypeRegistry*>(typeRegistryRaw);

        const sierra::odyplan::v2::common::window::count::CountWindowConfig config{.size = static_cast<std::uint64_t>(size), .slide = static_cast<std::uint64_t>(slide)};

        sierra::odyplan::v2::core::logical::port::Port schema;
        for (std::uint64_t i = 0; i < fieldCount; ++i) {
            schema.push_back(sierra::odyplan::v2::core::logical::port::PortDescriptor{.typeId = typeIds[i]});
        }

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        return new sierra::odyplan::v2::common::window::count::CountWindowBuffer(config, std::move(schema), *typeRegistry);
    }

    inline void rtDestroyCountWindowBuffer(void* state) {
        auto* buffer = static_cast<sierra::odyplan::v2::common::window::IWindowBuffer*>(state);
        buffer->close();
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete buffer;
    }

    inline void* rtCreateTimeWindowBuffer(const void* typeRegistryRaw, const std::int64_t size, const std::int64_t slide,
                                    const std::int64_t allowedLateness, const std::uint8_t* typeIds, const std::uint64_t fieldCount) {
        const auto* typeRegistry = static_cast<const sierra::odyplan::v2::core::common::type::TypeRegistry*>(typeRegistryRaw);

        const sierra::odyplan::v2::common::window::time::TimeWindowConfig config{
            .size = std::chrono::milliseconds(size),
            .slide = std::chrono::milliseconds(slide),
            .allowedLateness = std::chrono::milliseconds(allowedLateness)
        };

        sierra::odyplan::v2::core::logical::port::Port schema;
        for (std::uint64_t i = 0; i < fieldCount; ++i) {
            schema.push_back(sierra::odyplan::v2::core::logical::port::PortDescriptor{.typeId = typeIds[i]});
        }

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        return new sierra::odyplan::v2::common::window::time::TimeWindowBuffer(config, std::move(schema), *typeRegistry);
    }

    inline void rtDestroyTimeWindowBuffer(void* state) {
        auto* buffer = static_cast<sierra::odyplan::v2::common::window::IWindowBuffer*>(state);
        buffer->close();
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete buffer;
    }

    inline void rtWindowBeginRow(void* state) {
        static_cast<sierra::odyplan::v2::common::window::IWindowBuffer*>(state)->begin();
    }

    inline void rtWindowPush(void* state, const std::uint64_t fieldIndex, const void* rawData) {
        static_cast<sierra::odyplan::v2::common::window::IWindowBuffer*>(state)->push(fieldIndex, rawData);
    }

    inline void rtWindowEndRow(void* state, const std::int64_t timestamp) {
        static_cast<sierra::odyplan::v2::common::window::IWindowBuffer*>(state)->end(std::chrono::sys_time(std::chrono::milliseconds(timestamp)));
    }

    inline void rtWindowCompute(void* state) {
        static_cast<sierra::odyplan::v2::common::window::IWindowBuffer*>(state)->compute();
    }

    inline void rtWindowFlush(void* state) {
        static_cast<sierra::odyplan::v2::common::window::IWindowBuffer*>(state)->flush();
    }

    inline std::uint64_t rtWindowGetCount(void* state) {
        return static_cast<sierra::odyplan::v2::common::window::IWindowBuffer*>(state)->getWindowCount();
    }

    inline std::uint64_t rtWindowGetRowCountInWindow(void* state, const std::uint64_t windowIndex) {
        return static_cast<sierra::odyplan::v2::common::window::IWindowBuffer*>(state)->getRowCountInWindow(windowIndex);
    }

    inline const void* rtWindowGetData(void* state, const uint64_t windowIndex, const uint64_t rowIndex, const uint64_t fieldIndex) {
        return static_cast<sierra::odyplan::v2::common::window::IWindowBuffer*>(state)->getData(windowIndex, rowIndex, fieldIndex);
    }

    inline std::int64_t rtWindowGetTimestamp(void* state, const std::uint64_t windowIndex, const std::uint64_t rowIndex) {
        const auto timestamp = static_cast<sierra::odyplan::v2::common::window::IWindowBuffer*>(state)->getTimestamp(windowIndex, rowIndex);
        return timestamp.time_since_epoch().count();
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_OPERATOR_RUNTIME_H