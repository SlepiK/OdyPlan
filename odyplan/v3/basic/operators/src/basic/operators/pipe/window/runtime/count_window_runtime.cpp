#include <basic/operators/pipe/window/runtime/count_window_runtime.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::runtime {
    extern "C" {
    void* rtCreateCountWindowBuffer(const void* typeRegistryRaw,
                                    const std::int64_t size,
                                    const std::int64_t slide,
                                    const std::int64_t advance,
                                    const core::shared::types::ValueTypeId* typeIds,
                                    const std::uint64_t fieldCount) {
        const auto* runtimeTypeRegistry =
            static_cast<const core::runtime::types::RuntimeTypeRegistry*>(typeRegistryRaw);

        const basic::window::count::CountWindowConfig config{
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

    void rtCountWindowBeginRow(void *state) {
        static_cast<basic::window::count::ICountWindowBuffer*>(state)->begin();
    }

    void rtCountWindowPush(void *state, const std::uint64_t fieldIndex, const void *rawData) {
        static_cast<basic::window::count::ICountWindowBuffer*>(state)->push(fieldIndex, rawData);
    }

    void rtCountWindowEndRow(void *state, const std::int64_t startTs, const std::int64_t endTs) {
        static_cast<basic::window::count::ICountWindowBuffer*>(state)->end(startTs, endTs);
    }

    bool rtCountWindowReady(void *state) {
        return static_cast<basic::window::count::ICountWindowBuffer*>(state)->ready();
    }

    std::uint64_t rtCountWindowRowCount(void *state) {
        return static_cast<basic::window::count::ICountWindowBuffer*>(state)->getRowCount();
    }

    const void *rtCountWindowGetData(void *state, const std::uint64_t rowIndex, const std::uint64_t fieldIndex) {
        return static_cast<basic::window::count::ICountWindowBuffer*>(state)->getData(rowIndex, fieldIndex);
    }

    std::int64_t rtCountWindowRowStart(void *state, const std::uint64_t rowIndex) {
        return static_cast<basic::window::count::ICountWindowBuffer*>(state)->getRowStart(rowIndex);
    }

    std::int64_t rtCountWindowEnd(void *state) {
        return static_cast<basic::window::count::ICountWindowBuffer*>(state)->getWindowEnd();
    }

    void rtCountWindowAdvance(void *state) {
        static_cast<basic::window::count::ICountWindowBuffer*>(state)->advanceWindow();
    }

    void rtCountWindowFlush(void *state) {
        static_cast<basic::window::count::ICountWindowBuffer*>(state)->flush();
    }

    std::uint64_t rtCountWindowEmitCount(void* state) {
        return static_cast<basic::window::count::ICountWindowBuffer*>(state)->getEmitCount();
    }
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::runtime