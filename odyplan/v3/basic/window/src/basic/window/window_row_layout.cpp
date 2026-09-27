#include <basic/window/window_row_layout.h>
#include <cstring>

namespace sierra::odyplan::v3::basic::window {
    WindowRowLayout::WindowRowLayout(
        core::logical::port::Port dataSchema,
        const core::runtime::types::RuntimeTypeRegistry& runtimeTypeRegistry)
        : mDataSchema(std::move(dataSchema)), mRuntimeTypeRegistry(&runtimeTypeRegistry) {
        std::size_t offset = 0;
        mOffset.reserve(mDataSchema.size());
        for (const auto& dataSchemaField : mDataSchema) {
            mOffset.push_back(offset);
            offset += mRuntimeTypeRegistry->byteSize(dataSchemaField.valueTypeId);
        }
        mRowByteSize = offset;
    }

    void WindowRowLayout::begin() {
        mCurrentRow.assign(mRowByteSize, std::byte{0});
    }

    void WindowRowLayout::push(const std::size_t index, const void* rawData) {
        const auto typeId = mDataSchema.at(index).valueTypeId;
        void* dst = mCurrentRow.data() + mOffset.at(index);
        if (mRuntimeTypeRegistry->isPointerType(typeId)) {
            void* ori = nullptr;
            std::memcpy(static_cast<void*>(&ori), rawData, sizeof(ori));
            const void* own = mRuntimeTypeRegistry->retain(typeId, ori);
            std::memcpy(dst, static_cast<void*>(&own), sizeof(own));
        } else {
            std::memcpy(dst, rawData, mRuntimeTypeRegistry->byteSize(typeId));
        }
    }

    std::vector<std::byte> WindowRowLayout::pop() {
        return std::exchange(mCurrentRow, {});
    }

    std::size_t WindowRowLayout::offsetOf(const std::size_t field) const {
        return mOffset.at(field);
    }

    void WindowRowLayout::releaseRow(const std::vector<std::byte>& row) const {
        for (std::size_t i = 0; i < mDataSchema.size(); ++i) {
            const auto typeId = mDataSchema.at(i).valueTypeId;
            if (!mRuntimeTypeRegistry->isPointerType(typeId)) {
                continue;
            }
            void* own = nullptr;
            std::memcpy(static_cast<void*>(&own), row.data() + mOffset.at(i), sizeof(own));
            mRuntimeTypeRegistry->release(typeId, own);
        }
    }

    std::size_t WindowRowLayout::getTotalByteSize() const noexcept {
        return mRowByteSize;
    }

    std::size_t WindowRowLayout::getFieldCount() const noexcept {
        return mDataSchema.size();
    }

    core::shared::types::ValueTypeId WindowRowLayout::getFieldTypeId(std::size_t index) const {
        return mDataSchema.at(index).valueTypeId;
    }

    void WindowRowLayout::discardCurrentRow() {
        releaseRow(mCurrentRow);
        mCurrentRow.clear();
    }

    const std::byte* WindowRowLayout::getCurrentRowPtr() const {
        return mCurrentRow.data();
    }

    std::size_t WindowRowLayout::getCurrentRowSize() const {
        return mCurrentRow.size();
    }

    const core::logical::port::Port& WindowRowLayout::getSchema() const {
        return mDataSchema;
    }
}  // namespace sierra::odyplan::v3::basic::window