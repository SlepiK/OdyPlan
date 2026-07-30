#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_INTERVAL_JOIN_OPERATOR_PHYSICAL_INTERNAL_SIDE_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_INTERVAL_JOIN_OPERATOR_PHYSICAL_INTERNAL_SIDE_H
#include <chrono>
#include <deque>

#include "common/window/window_row_layout.h"
#include "core/common/type/type_registry.h"
#include "core/logical/port/port.h"

namespace sierra::odyplan::v2::common::opr::pipe::physical::internal {

    using Timestamp = std::chrono::sys_time<std::chrono::milliseconds>;

    class IntervalJoinSide {
    public:
        IntervalJoinSide(core::logical::port::Port schema, const core::common::type::TypeRegistry& typeRegistry,
            const std::chrono::milliseconds windowSize)
                : mLayout(std::move(schema), typeRegistry),
                    mWindowSize(windowSize) {
        }

        void begin() {
            mLayout.begin();
        }

        void push(const std::size_t index, const void* data) {
            mLayout.push(index, data);
        }

        void evict(const Timestamp otherMark) {
            while (!mBufferedRows.empty() && (mBufferedRows.front().timestamp + mWindowSize) <= otherMark) {
                if (mKeyFieldIndex) {
                    const auto* keyPtr = mBufferedRows.front().values.data() + mLayout.offsetOf(*mKeyFieldIndex);
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

        void end(const Timestamp timestamp) {
            const auto absoluteIndex = mBasisIndex + mBufferedRows.size();

            mBufferedRows.push_back(RowBuffer{.values = mLayout.pop(), .timestamp = timestamp});

            if (mKeyFieldIndex) {
                const auto* keyPtr = mBufferedRows.back().values.data() + mLayout.offsetOf(*mKeyFieldIndex);
                mKeyIndex.emplace(mHashFunction(keyPtr), absoluteIndex);
            }

            mLastAbsoluteIndex = absoluteIndex;

            if (!mMaxTimeStamp || (mMaxTimeStamp.has_value() && timestamp > mMaxTimeStamp.value())) {
                mMaxTimeStamp = timestamp;
            }
        }

        void release() {
            for (auto&[values, timestampMs] : mBufferedRows) {
                mLayout.releaseRow(values);
            }
            mBufferedRows.clear();
            mKeyIndex.clear();
        }

        [[nodiscard]] std::size_t getSize() const {
            return mBufferedRows.size();
        }

        [[nodiscard]] std::optional<Timestamp> getMaxTimestamp() const {
            return mMaxTimeStamp;
        }

        [[nodiscard]] std::chrono::milliseconds getWindowSize() const {
            return mWindowSize;
        }

        [[nodiscard]] const std::byte* getFieldPtr(const std::size_t rowIndex, const std::size_t fieldIndex) const {
            return mBufferedRows.at(rowIndex - mBasisIndex).values.data() + mLayout.offsetOf(fieldIndex);
        }

        [[nodiscard]] bool isOver(const Timestamp otherTimestamp, const std::chrono::milliseconds otherWindowSize, const std::size_t rowIndex) const {
            const auto&[values, timestamp] = mBufferedRows.at(rowIndex - mBasisIndex);
            return (otherTimestamp < (timestamp + mWindowSize)) && (timestamp < (otherTimestamp + otherWindowSize));
        }

        [[nodiscard]] Timestamp getTimestamp(const std::size_t rowIndex) const {
            return mBufferedRows.at(rowIndex - mBasisIndex).timestamp;
        }

        void enableKeyIdx(const std::size_t fieldIndex, core::common::type::TypeRegistry::HashFn hashFunction,
            core::common::type::TypeRegistry::EqualFn equalFunction) {
            mKeyFieldIndex = fieldIndex;
            mHashFunction = std::move(hashFunction);
            mEqualFunction = std::move(equalFunction);
        }

        [[nodiscard]] std::optional<std::size_t> getKeyFieldIndex() const {
            return mKeyFieldIndex;
        }

        [[nodiscard]] bool hasKeyIndex() const {
            return mKeyFieldIndex.has_value();
        }

        [[nodiscard]] std::uint64_t getLastAbsoluteIndex() const {
            return mLastAbsoluteIndex;
        }

        [[nodiscard]] std::vector<std::uint64_t> hashLookup(const std::byte* query) {
            std::vector<std::uint64_t> result;
            const auto hashValue = mHashFunction(query);
            const auto [z, h] = mKeyIndex.equal_range(hashValue);
            for (auto iter = z; iter != h; ++iter) {
                const auto candidateIndex = iter->second;
                const auto* candidate = getFieldPtr(candidateIndex, *mKeyFieldIndex);
                if (mEqualFunction(candidate, query)) {
                    result.push_back(candidateIndex);
                }
            }
            return result;
        }

        [[nodiscard]] std::uint64_t getBaseIndex() const {
            return mBasisIndex;
        }

    private:
        struct RowBuffer {
            std::vector<std::byte> values;
            Timestamp timestamp;
        };

        window::WindowRowLayout mLayout;
        std::chrono::milliseconds mWindowSize;
        std::deque<RowBuffer> mBufferedRows;
        std::optional<Timestamp> mMaxTimeStamp;
        //Hash-Behavior for == predicate only
        std::optional<std::size_t> mKeyFieldIndex;
        std::unordered_multimap<std::size_t, std::uint64_t> mKeyIndex;
        core::common::type::TypeRegistry::HashFn mHashFunction;
        core::common::type::TypeRegistry::EqualFn mEqualFunction;

        std::uint64_t mBasisIndex = 0;
        std::uint64_t mLastAbsoluteIndex = 0;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_INTERVAL_JOIN_OPERATOR_PHYSICAL_INTERNAL_SIDE_H