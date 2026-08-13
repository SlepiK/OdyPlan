#ifndef SIERRA_ODYPLAN_V3_BASIC_WINDOW_WINDOW_BUFFER_HPP
#define SIERRA_ODYPLAN_V3_BASIC_WINDOW_WINDOW_BUFFER_HPP

#include <chrono>

namespace sierra::odyplan::v3::basic::window {
    class IWindowBuffer {
    public:
        using Timestamp = std::chrono::sys_time<std::chrono::milliseconds>;

        IWindowBuffer() = default;
        virtual ~IWindowBuffer() = default;
        IWindowBuffer(IWindowBuffer const&) = delete;
        IWindowBuffer& operator=(IWindowBuffer const&) = delete;
        IWindowBuffer(IWindowBuffer&&) = delete;
        IWindowBuffer& operator=(IWindowBuffer&&) = delete;

        virtual void begin() = 0;

        virtual void close() = 0;

        virtual void push(std::size_t index, const void* rawData) = 0;

        virtual void end(std::chrono::sys_time<std::chrono::milliseconds> timestamp) = 0;

        virtual void compute() = 0;

        virtual void flush() = 0;

        [[nodiscard]] virtual std::size_t getWindowCount() const = 0;

        [[nodiscard]] virtual std::size_t getRowCountInWindow(std::size_t index) const = 0;

        [[nodiscard]] virtual const std::byte*
        getData(std::size_t windowIndex, std::size_t rowIndex, std::size_t fieldIndex) const = 0;

        [[nodiscard]] virtual std::chrono::sys_time<std::chrono::milliseconds>
        getTimestamp(std::size_t windowIndex, std::size_t rowIndex) const = 0;

        [[nodiscard]] virtual std::chrono::sys_time<std::chrono::milliseconds>
        getWindowStart(std::size_t windowIndex) const = 0;

        [[nodiscard]] virtual std::optional<std::chrono::sys_time<std::chrono::milliseconds>>
        getWindowEnd(std::size_t windowIndex) const = 0;
    };
}  // namespace sierra::odyplan::v3::basic::window

#endif  // SIERRA_ODYPLAN_V3_BASIC_WINDOW_WINDOW_BUFFER_HPP