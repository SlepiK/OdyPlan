#ifndef SIERRA_ODYPLAN_V3_BASIC_STORAGE_BUFFER_PUNCTUATION_BUFFER_H
#define SIERRA_ODYPLAN_V3_BASIC_STORAGE_BUFFER_PUNCTUATION_BUFFER_H
#include <algorithm>
#include <vector>

#include "core/shared/types/global_types.h"

namespace sierra::odyplan::v3::basic::storage::buffer {
    struct PunctuationBufferEntry {
        core::shared::types::Timestamp mark;
        bool isFinal;
        bool isHeartbeat;
    };

    class PunctuationBuffer {
    public:
        PunctuationBuffer() = default;

        virtual ~PunctuationBuffer() = default;

        void store(const PunctuationBufferEntry &punc) {
            mBuffer.push_back(punc);
        }

        std::size_t flush(const core::shared::types::Timestamp minTs) {
            mToSendIndices.clear();

            for (std::size_t i = 0; i < mBuffer.size(); ++i) {
                if (mBuffer[i].mark < minTs) {
                    mToSendIndices.push_back(i);
                }
            }

            return mToSendIndices.size();
        }

        [[nodiscard]] const PunctuationBufferEntry &getEntryByIndex(const std::size_t index) const {
            return mBuffer[mToSendIndices[index]];
        }

        void cleanup() {
            std::ranges::sort(mToSendIndices);
            for (const auto index: mToSendIndices) {
                mBuffer.erase(mBuffer.begin() + index);
            }
            mToSendIndices.clear();
        }

        [[nodiscard]] std::size_t size() const {
            return mBuffer.size();
        }

        [[nodiscard]] std::size_t toSendCount() const {
            return mToSendIndices.size();
        }

    private:
        std::vector<PunctuationBufferEntry> mBuffer;
        std::vector<std::size_t> mToSendIndices;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_STORAGE_BUFFER_PUNCTUATION_BUFFER_H