#include <basic/operators/pipe/join/physical/details/join_side.h>

namespace sierra::odyplan::v3::basic::operators::pipe::join::physical::details {
    JoinSide::JoinSide(core::logical::port::Port schema,
                       const core::physical::types::ValueTypeRegistry& typeRegistry,
                       const std::chrono::milliseconds windowSize)
        : mLayout(std::move(schema), typeRegistry), mWindowSize(windowSize) {}

    void JoinSide::begin() {
        mLayout.begin();
    }

    void JoinSide::push(const std::size_t index, const void* data) {
        mLayout.push(index, data);
    }

    void JoinSide::evict(const Timestamp otherMark) {
        while (!mBufferedRows.empty() &&
               (mBufferedRows.front().timestamp + mWindowSize) <= otherMark) {
            if (mKeyFieldIndex) {
                const auto* keyPtr =
                    mBufferedRows.front().values.data() + mLayout.offsetOf(*mKeyFieldIndex);
                const auto hashValue = mHashFunction(keyPtr);
                auto [v, g] = mKeyIndex.equal_range(hashValue);
                for (auto iter = v; iter != g; ++iter) {
                    if (iter->second == mBasisIndex) {
                        mKeyIndex.erase(iter);
                        break;
                    }
                }
            }
            mLayout.releaseRow(mBufferedRows.front().values);
            mBufferedRows.pop_front();
            ++mBasisIndex;
        }
    }

    void JoinSide::end(const Timestamp timestamp) {
        const auto absoluteIndex = mBasisIndex + mBufferedRows.size();

        mBufferedRows.push_back(RowBuffer{.values = mLayout.pop(), .timestamp = timestamp});

        if (mKeyFieldIndex) {
            const auto* keyPtr =
                mBufferedRows.back().values.data() + mLayout.offsetOf(*mKeyFieldIndex);
            mKeyIndex.emplace(mHashFunction(keyPtr), absoluteIndex);
        }

        mLastAbsoluteIndex = absoluteIndex;

        if (!mMaxTimeStamp || (mMaxTimeStamp.has_value() && timestamp > mMaxTimeStamp.value())) {
            mMaxTimeStamp = timestamp;
        }
    }

    void JoinSide::release() {
        for (auto& [values, timestampMs] : mBufferedRows) {
            mLayout.releaseRow(values);
        }
        mBufferedRows.clear();
        mKeyIndex.clear();
    }

    std::size_t JoinSide::getSize() const {
        return mBufferedRows.size();
    }

    std::optional<Timestamp> JoinSide::getMaxTimestamp() const {
        return mMaxTimeStamp;
    }

    std::chrono::milliseconds JoinSide::getWindowSize() const {
        return mWindowSize;
    }

    const std::byte* JoinSide::getFieldPtr(const std::size_t rowIndex,
                                           const std::size_t fieldIndex) const {
        return mBufferedRows.at(rowIndex - mBasisIndex).values.data() +
               mLayout.offsetOf(fieldIndex);
    }

    bool JoinSide::isOver(const Timestamp otherTimestamp,
                          const std::chrono::milliseconds otherWindowSize,
                          const std::size_t rowIndex) const {
        const auto& [values, timestamp] = mBufferedRows.at(rowIndex - mBasisIndex);
        return (otherTimestamp < (timestamp + mWindowSize)) &&
               (timestamp < (otherTimestamp + otherWindowSize));
    }

    Timestamp JoinSide::getTimestamp(const std::size_t rowIndex) const {
        return mBufferedRows.at(rowIndex - mBasisIndex).timestamp;
    }

    void JoinSide::enableKeyIdx(const std::size_t fieldIndex,
                                core::physical::types::ValueTypeRegistry::HashFn hashFunction,
                                core::physical::types::ValueTypeRegistry::EqualFn equalFunction) {
        mKeyFieldIndex = fieldIndex;
        mHashFunction = std::move(hashFunction);
        mEqualFunction = std::move(equalFunction);
    }

    std::optional<std::size_t> JoinSide::getKeyFieldIndex() const {
        return mKeyFieldIndex;
    }

    bool JoinSide::hasKeyIndex() const {
        return mKeyFieldIndex.has_value();
    }

    std::uint64_t JoinSide::getLastAbsoluteIndex() const {
        return mLastAbsoluteIndex;
    }

    std::vector<std::uint64_t> JoinSide::hashLookup(const std::byte* query) {
        std::vector<std::uint64_t> result;
        const auto hashValue = mHashFunction(query);
        const auto [z, h] = mKeyIndex.equal_range(hashValue);
        for (auto iter = z; iter != h; ++iter) {
            const auto candidateIndex = iter->second;
            // NOLINTNEXTLINE(bugprone-unchecked-optional-access,-warnings-as-errors)
            const auto* candidate = getFieldPtr(candidateIndex, *mKeyFieldIndex);
            if (mEqualFunction(candidate, query)) {
                result.push_back(candidateIndex);
            }
        }
        return result;
    }

    std::uint64_t JoinSide::getBaseIndex() const {
        return mBasisIndex;
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::physical::details