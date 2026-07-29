#ifndef SIERRA_ODYPLAN_V2_COMMON_WINDOW_WINDOWBUFFER_H
#define SIERRA_ODYPLAN_V2_COMMON_WINDOW_WINDOWBUFFER_H

#include <chrono>

namespace sierra::odyplan::v2::common::window {
    class IWindowBuffer {
    public:
        IWindowBuffer() = default;
        virtual ~IWindowBuffer() = default;
        IWindowBuffer(IWindowBuffer const&) = delete;
        IWindowBuffer& operator =(IWindowBuffer const&) = delete;
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

        [[nodiscard]] virtual const std::byte* getData(std::size_t windowIndex, std::size_t rowIndex, std::size_t fieldIndex) const = 0;

        [[nodiscard]] virtual std::chrono::sys_time<std::chrono::milliseconds> getTimestamp(std::size_t windowIndex, std::size_t rowIndex) const = 0;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_WINDOW_WINDOWBUFFER_H