#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_DATA_DATA_PACKAGE_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_DATA_DATA_PACKAGE_HPP

#include <llvm/IR/Value.h>

#include <cstdint>
#include <vector>

namespace sierra::odyplan::v3::core::physical::data {
    struct WindowMetadata {
        //i64 start = inclusive, end = exclusive
        llvm::Value* windowStart = nullptr;
        llvm::Value* windowEnd = nullptr;
        llvm::Value* hasKnownEnd  = nullptr;
    };

    struct DataPackage {
        llvm::Value* timestamp = nullptr;

        std::optional<WindowMetadata> windowMetadata;

        std::vector<llvm::Value*> fields;

        [[nodiscard]] std::size_t size() const;

        [[nodiscard]] llvm::Value* at(std::size_t index) const;

        [[nodiscard]] llvm::Value* operator[](std::size_t index) const;

        [[nodiscard]] std::vector<llvm::Value*>::const_iterator begin() const;

        [[nodiscard]] std::vector<llvm::Value*>::const_iterator end() const;

        void pushBack(llvm::Value* value);

        void reserve(std::size_t size);

        [[nodiscard]] bool hasWindowMetadata() const noexcept;
    };
}  // namespace sierra::odyplan::v3::core::physical::data

#endif  // SIERRA_ODYPLAN_V3_CORE_PHYSICAL_DATA_DATA_PACKAGE_HPP