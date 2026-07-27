#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_OPERATOR_RUNTIME_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_OPERATOR_RUNTIME_H
#include <algorithm>
#include <memory>
#include <vector>

#include "common/window/window_buffer.h"

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
    inline void rtWindowBeginRow(const std::uint64_t handle) {
        gWindows.at(handle)->begin();
    }

    inline void rtWindowPush(const std::uint64_t handle, const std::uint64_t fieldIndex, const void* rawData) {
        gWindows.at(handle)->push(fieldIndex, rawData);
    }

    inline void rtWindowEndRow(const std::uint64_t handle, const std::int64_t timestamp) {
        gWindows.at(handle)->end(std::chrono::sys_time(std::chrono::milliseconds(timestamp)));
    }

    inline void rtWindowCompute(const std::uint64_t handle) {
        gWindows.at(handle)->compute();
    }

    inline void rtWindowFlush(const std::uint64_t handle) {
        gWindows.at(handle)->flush();
    }

    inline std::uint64_t rtWindowGetCount(const std::uint64_t handle) {
        return gWindows.at(handle)->getWindowCount();
    }

    inline std::uint64_t rtWindowGetRowCountInWindow(const std::uint64_t handle, const std::uint64_t windowIndex) {
        return gWindows.at(handle)->getRowCountInWindow(windowIndex);
    }

    inline const void* rtWindowGetData(const std::uint64_t handle, const uint64_t windowIndex, const uint64_t rowIndex, const uint64_t fieldIndex) {
        return gWindows.at(handle)->getData(windowIndex, rowIndex, fieldIndex);
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_OPERATOR_RUNTIME_H