#include <core/physical/data/data_package.h>

namespace sierra::odyplan::v3::core::physical::data {
    std::size_t DataPackage::size() const {
        return fields.size();
    }

    llvm::Value* DataPackage::at(const std::size_t index) const {
        return fields.at(index);
    }

    llvm::Value* DataPackage::operator[](const std::size_t index) const {
        return fields.at(index);
    }

    auto DataPackage::begin() const {
        return fields.begin();
    }

    auto DataPackage::end() const {
        return fields.end();
    }

    void DataPackage::pushBack(llvm::Value* value) {
        fields.push_back(value);
    }
    void DataPackage::reserve(const std::size_t size) {
        fields.reserve(size);
    }
}