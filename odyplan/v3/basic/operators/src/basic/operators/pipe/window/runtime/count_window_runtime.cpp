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

    void *rtCreatePunctuationBuffer() {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        return new CountWindowPunctuationBuffer();
    }

    void rtPunctuationBufferStore(void* buffer, core::shared::types::Timestamp watermark, bool isFinal, bool isHeartbeat) {
        auto* buf = static_cast<CountWindowPunctuationBuffer*>(buffer);
        CountWindowPunctuationBuffer::CountWindowPunctuationBufferEntry const entry{
            .mark = watermark,
            .isFinal = isFinal,
            .isHeartbeat = isHeartbeat
        };
        buf->store(entry);
    }

    std::size_t rtPunctuationBufferFlush(void *buffer, core::shared::types::Timestamp minTs) {
        auto* buf = static_cast<CountWindowPunctuationBuffer*>(buffer);
        return buf->flush(minTs);
    }

    const void *rtPunctuationBufferGetToSend(void *buffer, std::size_t index) {
        auto *buf = static_cast<CountWindowPunctuationBuffer *>(buffer);
        return &buf->getToSend(index);
    }

    void rtPunctuationBufferCleanup(void *buffer) {
        auto *buf = static_cast<CountWindowPunctuationBuffer *>(buffer);
        buf->cleanup();
    }

    void rtDestroyPunctuationBuffer(void *buffer) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete static_cast<CountWindowPunctuationBuffer*>(buffer);
    }
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::runtime