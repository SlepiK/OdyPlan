#include <basic/operators/pipe/window/runtime/count_window_runtime.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::runtime {
    extern "C" {
    void* rtCreateCountWindowBuffer(const void* typeRegistryRaw,
                                    const std::uint8_t windowType,
                                    const std::int64_t size,
                                    const std::int64_t slide,
                                    const std::int64_t advance,
                                    const core::shared::types::ValueTypeId* typeIds,
                                    const std::uint64_t fieldCount) {
        const auto* runtimeTypeRegistry =
            static_cast<const core::runtime::types::RuntimeTypeRegistry*>(typeRegistryRaw);

        const basic::window::count::CountWindowConfig config{
            .type = static_cast<basic::window::WindowType>(windowType),
            .size = static_cast<std::uint64_t>(size),
            .slide = static_cast<std::uint64_t>(slide),
            .advance = static_cast<std::uint64_t>(advance)
        };

        core::logical::port::Port schema;
        for (std::uint64_t i = 0; i < fieldCount; ++i) {
            schema.push_back(core::logical::port::PortDescription{.valueTypeId = typeIds[i]});
        }

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        return new basic::window::count::CountWindowBuffer(
            config, std::move(schema), *runtimeTypeRegistry);
    }

    void rtDestroyCountWindowBuffer(void* state) {
        auto* buffer = static_cast<basic::window::count::ICountWindowBuffer*>(state);
        buffer->close();
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete buffer;
    }

    void rtCountWindowBeginRow(void* state) {
        static_cast<basic::window::count::ICountWindowBuffer*>(state)->begin();
    }

    void rtCountWindowPush(void* state, const std::uint64_t fieldIndex, const void* rawData) {
        static_cast<basic::window::count::ICountWindowBuffer*>(state)->push(fieldIndex, rawData);
    }

    void rtCountWindowEndRow(void* state, const std::int64_t timestamp) {
        static_cast<basic::window::count::ICountWindowBuffer*>(state)->end(
            std::chrono::sys_time(std::chrono::milliseconds(timestamp)));
    }

    void rtCountWindowCompute(void* state) {
        static_cast<basic::window::count::ICountWindowBuffer*>(state)->compute();
    }

    void rtCountWindowFlush(void* state) {
        static_cast<basic::window::count::ICountWindowBuffer*>(state)->flush();
    }

    std::int64_t rtCountWindowGetStart(void* state, const std::uint64_t windowIndex) {
        return static_cast<basic::window::count::ICountWindowBuffer *>(state)->getWindowStart(
            static_cast<std::size_t>(windowIndex)).time_since_epoch().count();
    }

    std::int64_t rtCountWindowGetStartRow(void *state, const std::uint64_t windowIndex, const std::uint64_t rowIndex) {
        return static_cast<basic::window::count::ICountWindowBuffer *>(state)->getWindowStartRow(
            static_cast<std::size_t>(windowIndex), static_cast<std::size_t>(rowIndex)).time_since_epoch().count();
    }

    std::int64_t rtCountWindowGetEnd(void* state, const std::uint64_t windowIndex, bool* outHasEnd) {
        auto* buffer = static_cast<basic::window::count::ICountWindowBuffer*>(state);
        const auto end = buffer->getWindowEnd(static_cast<std::size_t>(windowIndex));
        *outHasEnd = end.has_value();
        return end.has_value() ? end->time_since_epoch().count() : 0;
    }

    std::uint64_t rtCountWindowGetCount(void* state) {
        return static_cast<basic::window::count::ICountWindowBuffer*>(state)->getWindowCount();
    }

    std::uint64_t rtCountWindowGetRowCountInWindow(void* state, const std::uint64_t windowIndex) {
        return static_cast<basic::window::count::ICountWindowBuffer*>(state)->getRowCountInWindow(windowIndex);
    }

    const void* rtCountWindowGetData(void* state,
                                const uint64_t windowIndex,
                                const uint64_t rowIndex,
                                const uint64_t fieldIndex) {
        return static_cast<basic::window::count::ICountWindowBuffer*>(state)->getData(
            windowIndex, rowIndex, fieldIndex);
    }

    std::int64_t rtCountWindowGetTimestamp(void* state,
                                      const std::uint64_t windowIndex,
                                      const std::uint64_t rowIndex) {
        const auto timestamp =
            static_cast<basic::window::count::ICountWindowBuffer*>(state)->getTimestamp(windowIndex, rowIndex);
        return timestamp.time_since_epoch().count();
    }

    bool rtCountWindowIsClosedByFlush(void* state, const std::uint64_t windowIndex) {
        return static_cast<basic::window::count::ICountWindowBuffer*>(state)->isFlushedClosed(windowIndex);
    }
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::runtime