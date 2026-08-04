#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_DATA_DATA_PACKAGE_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_DATA_DATA_PACKAGE_HPP

#include <vector>
#include <cstdint>
#include <llvm/IR/Value.h>

namespace sierra::odyplan::v3::core::physical::data {
    struct DataPackage {
        llvm::Value* timestamp = nullptr;
        std::vector<llvm::Value*> fields;

        [[nodiscard]] std::size_t size() const;

        [[nodiscard]] llvm::Value* at(std::size_t index) const;

        [[nodiscard]] llvm::Value* operator[](std::size_t index) const;

        [[nodiscard]] auto begin() const;

        [[nodiscard]] auto end() const;

        void pushBack(llvm::Value* value);

        void reserve(std::size_t size);
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_PHYSICAL_DATA_DATA_PACKAGE_HPP