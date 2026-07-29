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
                mLayout.releaseRow(mBufferedRows.front().values);
                mBufferedRows.pop_front();
            }
        }

        void end(const Timestamp timestamp) {
            mBufferedRows.push_back(RowBuffer{.values = mLayout.pop(), .timestamp = timestamp});

            if (!mMaxTimeStamp || (mMaxTimeStamp.has_value() && timestamp > mMaxTimeStamp.value())) {
                mMaxTimeStamp = timestamp;
            }
        }

        void release() {
            for (auto&[values, timestampMs] : mBufferedRows) {
                mLayout.releaseRow(values);
            }
            mBufferedRows.clear();
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
            return mBufferedRows.at(rowIndex).values.data() + mLayout.offsetOf(fieldIndex);
        }

        [[nodiscard]] bool isOver(const Timestamp otherTimestamp, const std::chrono::milliseconds otherWindowSize, const std::size_t index) const {
            const auto&[values, timestamp] = mBufferedRows.at(index);
            return (otherTimestamp < (timestamp + mWindowSize)) && (timestamp < (otherTimestamp + otherWindowSize));
        }

        [[nodiscard]] Timestamp getTimestamp(const std::size_t rowIndex) const {
            return mBufferedRows.at(rowIndex).timestamp;
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
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_INTERVAL_JOIN_OPERATOR_PHYSICAL_INTERNAL_SIDE_H