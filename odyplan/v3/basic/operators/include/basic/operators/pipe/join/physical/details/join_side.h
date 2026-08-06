#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_DETAILS_JOIN_SIDE_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_DETAILS_JOIN_SIDE_HPP
#include <chrono>
#include <deque>

#include <basic/window/window_row_layout.h>
#include <core/physical/types/value_type_registry.h>
#include <core/logical/port/port.h>

namespace sierra::odyplan::v3::basic::operators::pipe::join::physical::details {
    using Timestamp = std::chrono::sys_time<std::chrono::milliseconds>;

    class JoinSide {
    public:
        JoinSide(core::logical::port::Port schema, const core::physical::types::ValueTypeRegistry& typeRegistry,
            std::chrono::milliseconds windowSize);

        void begin();

        void push(std::size_t index, const void* data);

        void evict(Timestamp otherMark);

        void end(Timestamp timestamp);

        void release();

        [[nodiscard]] std::size_t getSize() const;

        [[nodiscard]] std::optional<Timestamp> getMaxTimestamp() const;

        [[nodiscard]] std::chrono::milliseconds getWindowSize() const;

        [[nodiscard]] const std::byte* getFieldPtr(std::size_t rowIndex, std::size_t fieldIndex) const;

        [[nodiscard]] bool isOver(Timestamp otherTimestamp, std::chrono::milliseconds otherWindowSize, std::size_t rowIndex) const;

        [[nodiscard]] Timestamp getTimestamp(std::size_t rowIndex) const;

        void enableKeyIdx(std::size_t fieldIndex, core::physical::types::ValueTypeRegistry::HashFn hashFunction,
            core::physical::types::ValueTypeRegistry::EqualFn equalFunction);

        [[nodiscard]] std::optional<std::size_t> getKeyFieldIndex() const;

        [[nodiscard]] bool hasKeyIndex() const;

        [[nodiscard]] std::uint64_t getLastAbsoluteIndex() const;

        [[nodiscard]] std::vector<std::uint64_t> hashLookup(const std::byte* query);

        [[nodiscard]] std::uint64_t getBaseIndex() const;

    private:
        struct RowBuffer {
            std::vector<std::byte> values;
            Timestamp timestamp;
        };

        basic::window::WindowRowLayout mLayout;
        std::chrono::milliseconds mWindowSize;
        std::deque<RowBuffer> mBufferedRows;
        std::optional<Timestamp> mMaxTimeStamp;
        //Hash-Behavior for == predicate only
        std::optional<std::size_t> mKeyFieldIndex;
        std::unordered_multimap<std::size_t, std::uint64_t> mKeyIndex;
        core::physical::types::ValueTypeRegistry::HashFn mHashFunction;
        core::physical::types::ValueTypeRegistry::EqualFn mEqualFunction;

        std::uint64_t mBasisIndex = 0;
        std::uint64_t mLastAbsoluteIndex = 0;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_DETAILS_JOIN_SIDE_HPP