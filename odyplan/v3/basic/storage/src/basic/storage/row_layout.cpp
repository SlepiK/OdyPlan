#include <basic/storage/row_layout.h>
#include <cstring>

namespace sierra::odyplan::v3::basic::storage {
    RowLayout::RowLayout(
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

    void RowLayout::begin() {
        mCurrentRow.assign(mRowByteSize, std::byte{0});
    }

    void RowLayout::push(const std::size_t index, const void* rawData) {
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

    std::vector<std::byte> RowLayout::pop() {
        return std::exchange(mCurrentRow, {});
    }

    std::size_t RowLayout::offsetOf(const std::size_t field) const {
        return mOffset.at(field);
    }

    void RowLayout::releaseRow(const std::vector<std::byte>& row) const {
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

    std::size_t RowLayout::getTotalByteSize() const noexcept {
        return mRowByteSize;
    }

    std::size_t RowLayout::getFieldCount() const noexcept {
        return mDataSchema.size();
    }

    core::shared::types::ValueTypeId RowLayout::getFieldTypeId(std::size_t index) const {
        return mDataSchema.at(index).valueTypeId;
    }

    void RowLayout::discardCurrentRow() {
        releaseRow(mCurrentRow);
        mCurrentRow.clear();
    }

    const std::byte* RowLayout::getCurrentRowPtr() const {
        return mCurrentRow.data();
    }

    std::size_t RowLayout::getCurrentRowSize() const {
        return mCurrentRow.size();
    }

    const core::logical::port::Port& RowLayout::getSchema() const {
        return mDataSchema;
    }
}  // namespace sierra::odyplan::v3::basic::window