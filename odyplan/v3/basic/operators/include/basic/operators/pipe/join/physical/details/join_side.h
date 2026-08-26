#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_DETAILS_JOIN_SIDE_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_DETAILS_JOIN_SIDE_HPP
#include <basic/window/window_row_layout.h>
#include <chrono>
#include <core/logical/port/port.h>
#include <deque>
#include <map>

namespace sierra::odyplan::v3::basic::operators::pipe::join::physical::details {
    using Timestamp = std::chrono::sys_time<std::chrono::milliseconds>;

    class JoinSide {
    public:
        JoinSide(core::logical::port::Port schema,
                 const core::runtime::types::RuntimeTypeRegistry* runtimeTypeRegistry,
                 std::chrono::milliseconds windowSize);

        void begin();

        void push(std::size_t index, const void* data);

        void evict(Timestamp otherMark);

        void end(Timestamp timestamp);

        void release();

        [[nodiscard]] std::size_t getSize() const;

        [[nodiscard]] std::optional<Timestamp> getMaxTimestamp() const;

        [[nodiscard]] std::chrono::milliseconds getWindowSize() const;

        [[nodiscard]] const std::byte* getFieldPtr(std::size_t rowIndex,
                                                   std::size_t fieldIndex) const;

        [[nodiscard]] bool isOver(Timestamp otherTimestamp,
                                  std::chrono::milliseconds otherWindowSize,
                                  std::size_t rowIndex) const;

        [[nodiscard]] Timestamp getTimestamp(std::size_t rowIndex) const;

        void enableKeyIdx(const std::vector<std::size_t>& fieldIndices, const std::vector<core::shared::types::ValueTypeId>& keyTypeIds);

        [[nodiscard]] std::vector<std::size_t> getKeyFieldIndices() const;

        [[nodiscard]] bool hasKeyIndex() const;

        [[nodiscard]] std::uint64_t getLastAbsoluteIndex() const;

        [[nodiscard]] std::vector<std::uint64_t> hashLookup(const std::vector<const std::byte*>& query);

        [[nodiscard]] std::uint64_t getBaseIndex() const;

        [[nodiscard]] const std::byte* getRowPtr(std::size_t rowIndex) const;

        [[nodiscard]] std::size_t getRowByteSize() const;

        [[nodiscard]] std::size_t getFieldCount() const;

        [[nodiscard]] core::shared::types::ValueTypeId getFieldTypeId(std::size_t index) const;

        [[nodiscard]] std::size_t getFieldOffset(std::size_t index) const;

    private:
        struct RowBuffer {
            std::vector<std::byte> values;
            Timestamp timestamp;
        };

        struct KeyFieldDetails {
            std::size_t fieldIndex;
            core::runtime::types::RuntimeTypeRegistry::HashFunctionPtr mHashFunction;
            core::runtime::types::RuntimeTypeRegistry::EqualFunctionPtr mEqualFunction;
        };

        basic::window::WindowRowLayout mLayout;
        std::chrono::milliseconds mWindowSize;
        std::deque<RowBuffer> mBufferedRows;
        std::optional<Timestamp> mMaxTimeStamp;
        // Hash-Behavior for == predicate only

        std::multimap<std::vector<std::size_t>, std::uint64_t> mKeyIndex;
        std::vector<KeyFieldDetails> mKeyFields;
        const core::runtime::types::RuntimeTypeRegistry* mRuntimeTypeRegistry;

        std::uint64_t mBasisIndex = 0;
        std::uint64_t mLastAbsoluteIndex = 0;

        std::vector<std::uint64_t> computeRowKeyV(const std::byte* row) const;
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::physical::details

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_DETAILS_JOIN_SIDE_HPP