#include <basic/storage/buffer/punctuation_buffer_runtime.h>

#include "basic/storage/buffer/punctuation_buffer.h"

namespace sierra::odyplan::v3::basic::storage::buffer::runtime {
    extern "C" {
    void *rtCreatePunctuationBuffer() {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        return new storage::buffer::PunctuationBuffer();
    }

    void rtPunctuationBufferStore(void *buffer, const core::shared::types::Timestamp watermark, const bool isFinal,
                                  const bool isHeartbeat) {
        auto *buf = static_cast<storage::buffer::PunctuationBuffer *>(buffer);
        storage::buffer::PunctuationBufferEntry const entry{
            .mark = watermark,
            .isFinal = isFinal,
            .isHeartbeat = isHeartbeat
        };
        buf->store(entry);
    }

    std::size_t rtPunctuationBufferFlush(void *buffer, core::shared::types::Timestamp minTs) {
        auto *buf = static_cast<storage::buffer::PunctuationBuffer *>(buffer);
        return buf->flush(minTs);
    }

    const void *rtPunctuationBufferGetToSend(void *buffer, const std::size_t index) {
        const auto *buf = static_cast<storage::buffer::PunctuationBuffer *>(buffer);
        return &buf->getEntryByIndex(index);
    }

    void rtPunctuationBufferCleanup(void *buffer) {
        auto *buf = static_cast<storage::buffer::PunctuationBuffer *>(buffer);
        buf->cleanup();
    }

    void rtDestroyPunctuationBuffer(void *buffer) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete static_cast<storage::buffer::PunctuationBuffer *>(buffer);
    }
    }
}
