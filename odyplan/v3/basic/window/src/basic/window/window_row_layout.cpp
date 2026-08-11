#include <basic/window/window_row_layout.h>

namespace sierra::odyplan::v3::basic::window {
    WindowRowLayout::WindowRowLayout(core::logical::port::Port dataSchema,
                                     const core::physical::types::ValueTypeRegistry& typeRegistry)
        : mDataSchema(std::move(dataSchema)), mTypeRegistry(&typeRegistry) {
        std::size_t offset = 0;
        mOffset.reserve(mDataSchema.size());
        for (const auto& dataSchemaField : mDataSchema) {
            mOffset.push_back(offset);
            offset += mTypeRegistry->getByteSize(dataSchemaField.valueTypeId);
        }
        mRowByteSize = offset;
    }

    void WindowRowLayout::begin() {
        mCurrentRow.assign(mRowByteSize, std::byte{0});
    }

    void WindowRowLayout::push(const std::size_t index, const void* rawData) {
        const auto typeId = mDataSchema.at(index).valueTypeId;
        void* dst = mCurrentRow.data() + mOffset.at(index);
        if (mTypeRegistry->isPointerType(typeId)) {
            void* ori = nullptr;
            std::memcpy(static_cast<void*>(&ori), rawData, sizeof(ori));
            const void* own = mTypeRegistry->retain(typeId, ori);
            std::memcpy(dst, static_cast<void*>(&own), sizeof(own));
        } else {
            std::memcpy(dst, rawData, mTypeRegistry->getByteSize(typeId));
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
            if (!mTypeRegistry->isPointerType(typeId)) {
                continue;
            }
            void* own = nullptr;
            std::memcpy(static_cast<void*>(&own), row.data() + mOffset.at(i), sizeof(own));
            mTypeRegistry->release(typeId, own);
        }
    }

    void WindowRowLayout::discardCurrentRow() {
        releaseRow(mCurrentRow);
        mCurrentRow.clear();
    }
}  // namespace sierra::odyplan::v3::basic::window