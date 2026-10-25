#ifndef SIERRA_ODYPLAN_V3_BASIC_STORAGE_BUFFER_PUNCTUATION_BUFFER_RUNTIME_H
#define SIERRA_ODYPLAN_V3_BASIC_STORAGE_BUFFER_PUNCTUATION_BUFFER_RUNTIME_H
#include "core/shared/types/global_types.h"

namespace sierra::odyplan::v3::basic::storage::buffer::runtime {
    extern "C" {
    void *rtCreatePunctuationBuffer();

    void rtPunctuationBufferStore(void *buffer, core::shared::types::Timestamp watermark, bool isFinal,
                                  bool isHeartbeat);

    std::size_t rtPunctuationBufferFlush(void *buffer, core::shared::types::Timestamp minTs);

    const void *rtPunctuationBufferGetToSend(void *buffer, std::size_t index);

    void rtPunctuationBufferCleanup(void *buffer);

    void rtDestroyPunctuationBuffer(void *buffer);
    }
}

#endif //SIERRA_ODYPLAN_V3_BASIC_STORAGE_BUFFER_PUNCTUATION_BUFFER_RUNTIME_H