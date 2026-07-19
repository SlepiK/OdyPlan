#ifndef SIERRA_ODYPLAN_V2_COMMON_WINDOW_WINDOWROW_H
#define SIERRA_ODYPLAN_V2_COMMON_WINDOW_WINDOWROW_H
#include "core/common/type/type_registry.h"
#include "core/logical/port/port.h"

namespace sierra::odyplan::v2::common::window {
    class WindowRowLayout {
    public:
        WindowRowLayout(const core::logical::port::Port &dataSchema, const core::common::type::TypeRegistry& typeRegistry)
            : mDataSchema(std::move(dataSchema)),
                mTypeRegistry(&typeRegistry) {
            std::size_t offset = 0;
            mOffset.reserve(mDataSchema.size());
            for (const auto& dataSchemaField : mDataSchema) {
                mOffset.push_back(offset);
                offset += mTypeRegistry->getByteSize(dataSchemaField.typeId);
            }
            mRowByteSize = offset;
        }

        void begin() {
            mCurrentRow.assign(mRowByteSize, std::byte{0});
        }

        void push(const std::size_t index, const void* rawData) {
            const auto typeId = mDataSchema.at(index).typeId;
            void* dst = mCurrentRow.data() + mOffset.at(index);
            if (mTypeRegistry->isPointerType(typeId)) {
                void* ori;
                std::memcpy(&ori, rawData, sizeof(ori));
                void* own = mTypeRegistry->retain(typeId, ori);
                std::memcpy(dst, &own, sizeof(own));
            } else {
                std::memcpy(dst, rawData, mTypeRegistry->getByteSize(typeId));
            }
        }

        [[nodiscard]] std::vector<std::byte> pop() {
            return std::exchange(mCurrentRow, {});
        }

        [[nodiscard]] std::size_t offsetOf(const std::size_t field) const {
            return mOffset[field];
        }

        void releaseRow(const std::vector<std::byte>& row) const {
            for (std::size_t i = 0; i < mDataSchema.size(); ++i) {
                const auto typeId = mDataSchema.at(i).typeId;
                if (!mTypeRegistry->isPointerType(typeId)) {
                    continue;
                }
                void* own;
                std::memcpy(&own, row.data() + mOffset.at(i), sizeof(own));
                mTypeRegistry->release(typeId, own);
            }
        }

        void discardCurrentRow() {
            releaseRow(mCurrentRow);
            mCurrentRow.clear();
        }

    private:
        core::logical::port::Port mDataSchema;
        const core::common::type::TypeRegistry* mTypeRegistry;
        std::vector<std::size_t> mOffset;
        std::size_t mRowByteSize = 0;
        std::vector<std::byte> mCurrentRow;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_WINDOW_WINDOWROW_H