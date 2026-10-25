#include <basic/operators/pipe/window/runtime/count_window_runtime.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::runtime {
    extern "C" {
    void* rtCreateCountWindowBuffer(const std::size_t size,
                                    const std::size_t slide,
                                    const std::size_t advance,
                                    const core::shared::types::ValueTypeId* typeIds,
                                    const std::size_t fieldCount) {

        const details::CountWindowOptions config{
            .size = size,
            .slide = slide,
            .advance = advance
        };

        core::logical::port::Port schema;
        for (std::size_t i = 0; i < fieldCount; ++i) {
            schema.push_back(core::logical::port::PortDescription{.valueTypeId = typeIds[i]});
        }

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        return new CountWindowBuffer(config, std::move(schema));
    }

    void rtDestroyCountWindowBuffer(void* state) {
        auto* buffer = static_cast<ICountWindowBuffer*>(state);
        buffer->close();
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete buffer;
    }

    void rtCountWindowBeginRow(void *state) {
        static_cast<ICountWindowBuffer*>(state)->begin();
    }

    void rtCountWindowPush(void *state, const std::size_t fieldIndex, const void *rawData) {
        static_cast<ICountWindowBuffer*>(state)->push(fieldIndex, rawData);
    }

    void rtCountWindowEndRow(void *state, const core::shared::types::Timestamp startTs, const core::shared::types::Timestamp endTs) {
        static_cast<ICountWindowBuffer*>(state)->end(startTs, endTs);
    }

    bool rtCountWindowReady(void *state) {
        return static_cast<ICountWindowBuffer*>(state)->ready();
    }

    std::size_t rtCountWindowRowCount(void *state) {
        return static_cast<ICountWindowBuffer*>(state)->getRowCount();
    }

    const void *rtCountWindowGetData(void *state, const std::size_t rowIndex, const std::size_t fieldIndex) {
        return static_cast<ICountWindowBuffer*>(state)->getData(rowIndex, fieldIndex);
    }

    core::shared::types::Timestamp rtCountWindowRowStart(void *state, const std::size_t rowIndex) {
        return static_cast<ICountWindowBuffer*>(state)->getRowStart(rowIndex);
    }

    core::shared::types::Timestamp rtCountWindowEnd(void *state) {
        return static_cast<ICountWindowBuffer*>(state)->getWindowEnd();
    }

    void rtCountWindowAdvance(void *state) {
        static_cast<ICountWindowBuffer*>(state)->advanceWindow();
    }

    std::size_t rtCountWindowEmitCount(void* state) {
        return static_cast<ICountWindowBuffer*>(state)->getEmitCount();
    }

    core::shared::types::Timestamp rtCountWindowGetMinTs(void* state) {
        return static_cast<ICountWindowBuffer*>(state)->getMinTs();
    }
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::runtime