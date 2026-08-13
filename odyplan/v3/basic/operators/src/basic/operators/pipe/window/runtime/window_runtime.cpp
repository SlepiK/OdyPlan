#include <basic/operators/pipe/window/runtime/window_runtime.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::runtime {
    extern "C" {
    void* rtCreateCountWindowBuffer(const void* typeRegistryRaw,
                                    const std::int64_t size,
                                    const std::int64_t slide,
                                    const core::shared::types::ValueTypeId* typeIds,
                                    const std::uint64_t fieldCount) {
        const auto* runtimeTypeRegistry =
            static_cast<const core::runtime::types::RuntimeTypeRegistry*>(typeRegistryRaw);

        const basic::window::count::CountWindowConfig config{
            .size = static_cast<std::uint64_t>(size), .slide = static_cast<std::uint64_t>(slide)};

        core::logical::port::Port schema;
        for (std::uint64_t i = 0; i < fieldCount; ++i) {
            schema.push_back(core::logical::port::PortDescription{.valueTypeId = typeIds[i]});
        }

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        return new basic::window::count::CountWindowBuffer(
            config, std::move(schema), *runtimeTypeRegistry);
    }

    void rtDestroyCountWindowBuffer(void* state) {
        auto* buffer = static_cast<basic::window::IWindowBuffer*>(state);
        buffer->close();
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete buffer;
    }

    void rtDestroyTimeWindowBuffer(void* state) {
        auto* buffer = static_cast<basic::window::IWindowBuffer*>(state);
        buffer->close();
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete buffer;
    }

    void rtWindowBeginRow(void* state) {
        static_cast<basic::window::IWindowBuffer*>(state)->begin();
    }

    void rtWindowPush(void* state, const std::uint64_t fieldIndex, const void* rawData) {
        static_cast<basic::window::IWindowBuffer*>(state)->push(fieldIndex, rawData);
    }

    void rtWindowEndRow(void* state, const std::int64_t timestamp) {
        static_cast<basic::window::IWindowBuffer*>(state)->end(
            std::chrono::sys_time(std::chrono::milliseconds(timestamp)));
    }

    void rtWindowCompute(void* state) {
        static_cast<basic::window::IWindowBuffer*>(state)->compute();
    }

    void rtWindowFlush(void* state) {
        static_cast<basic::window::IWindowBuffer*>(state)->flush();
    }

    std::uint64_t rtWindowGetStart(void* state, std::int64_t windowIndex) {
        return static_cast<basic::window::IWindowBuffer*>(state)->getWindowStart(static_cast<std::size_t>(windowIndex)).time_since_epoch().count();
    }

    std::uint64_t rtWindowGetEnd(void* state, std::int64_t windowIndex) {
        return static_cast<basic::window::IWindowBuffer*>(state)->getWindowEnd(static_cast<std::size_t>(windowIndex)).time_since_epoch().count();
    }

    std::uint64_t rtWindowGetCount(void* state) {
        return static_cast<basic::window::IWindowBuffer*>(state)->getWindowCount();
    }

    std::uint64_t rtWindowGetRowCountInWindow(void* state, const std::uint64_t windowIndex) {
        return static_cast<basic::window::IWindowBuffer*>(state)->getRowCountInWindow(windowIndex);
    }

    const void* rtWindowGetData(void* state,
                                const uint64_t windowIndex,
                                const uint64_t rowIndex,
                                const uint64_t fieldIndex) {
        return static_cast<basic::window::IWindowBuffer*>(state)->getData(
            windowIndex, rowIndex, fieldIndex);
    }

    std::int64_t rtWindowGetTimestamp(void* state,
                                      const std::uint64_t windowIndex,
                                      const std::uint64_t rowIndex) {
        const auto timestamp =
            static_cast<basic::window::IWindowBuffer*>(state)->getTimestamp(windowIndex, rowIndex);
        return timestamp.time_since_epoch().count();
    }
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::runtime