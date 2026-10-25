#ifndef SIERRA_ODYPLAN_V3_BASIC_STORAGE_STORAGE_ROW_LAYOUT_HPP
#define SIERRA_ODYPLAN_V3_BASIC_STORAGE_STORAGE_ROW_LAYOUT_HPP
#include <core/logical/port/port.h>
#include <core/runtime/types/runtime_type_registry.h>
#include <utility>

namespace sierra::odyplan::v3::basic::storage::storage {
    class RowLayout {
    public:
        explicit RowLayout(core::logical::port::Port dataSchema);

        void begin();

        void push(std::size_t index, const void* rawData);

        [[nodiscard]] std::vector<std::byte> pop();

        [[nodiscard]] std::size_t offsetOf(std::size_t field) const;

        void releaseRow(const std::vector<std::byte>& row) const;

        void discardCurrentRow();

        [[nodiscard]] std::size_t getTotalByteSize() const noexcept;

        [[nodiscard]] std::size_t getFieldCount() const noexcept;

        [[nodiscard]] core::shared::types::ValueTypeId getFieldTypeId(std::size_t index) const;

        [[nodiscard]] const std::byte* getCurrentRowPtr() const;

        [[nodiscard]] std::size_t getCurrentRowSize() const;

        [[nodiscard]] const core::logical::port::Port& getSchema() const;
    private:
        const core::runtime::types::RuntimeTypeRegistry* mRegistry;
        core::logical::port::Port mDataSchema;
        std::vector<std::size_t> mOffset;
        std::size_t mRowByteSize = 0;
        std::vector<std::byte> mCurrentRow;
    };
}

#endif  // SIERRA_ODYPLAN_V3_BASIC_STORAGE_STORAGE_ROW_LAYOUT_HPP