#ifndef SIERRA_ODYPLAN_V3_BASIC_WINDOW_WINDOW_ROW_LAYOUT_HPP
#define SIERRA_ODYPLAN_V3_BASIC_WINDOW_WINDOW_ROW_LAYOUT_HPP
#include <core/logical/port/port.h>
#include <utility>

#include <core/runtime/types/runtime_type_registry.h>

namespace sierra::odyplan::v3::basic::window {
    class WindowRowLayout {
    public:
        WindowRowLayout(core::logical::port::Port dataSchema,
                        const core::runtime::types::RuntimeTypeRegistry& runtimeTypeRegistry);

        void begin();

        void push(std::size_t index, const void* rawData);

        [[nodiscard]] std::vector<std::byte> pop();

        [[nodiscard]] std::size_t offsetOf(std::size_t field) const;

        void releaseRow(const std::vector<std::byte>& row) const;

        void discardCurrentRow();

    private:
        core::logical::port::Port mDataSchema;
        const core::runtime::types::RuntimeTypeRegistry* mRuntimeTypeRegistry;
        std::vector<std::size_t> mOffset;
        std::size_t mRowByteSize = 0;
        std::vector<std::byte> mCurrentRow;
    };
}  // namespace sierra::odyplan::v3::basic::window

#endif  // SIERRA_ODYPLAN_V3_BASIC_WINDOW_WINDOW_ROW_LAYOUT_HPP