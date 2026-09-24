#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_DATA_DATA_PACKAGE_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_DATA_DATA_PACKAGE_HPP

#include <llvm/IR/Value.h>

#include <cstdint>
#include <vector>

#include <core/physical/time/time_interval.h>

namespace sierra::odyplan::v3::core::physical::data {
    struct DataPackage {
        time::TimeInterval timeInterval;

        std::vector<llvm::Value*> fields;

        DataPackage() = delete;

        explicit DataPackage(llvm::IRBuilder<>& builder);
        explicit DataPackage(const time::TimeInterval &ti);

        [[nodiscard]] std::size_t size() const;

        [[nodiscard]] llvm::Value* at(std::size_t index) const;

        [[nodiscard]] llvm::Value* operator[](std::size_t index) const;

        [[nodiscard]] std::vector<llvm::Value*>::const_iterator begin() const;

        [[nodiscard]] std::vector<llvm::Value*>::const_iterator end() const;

        void pushBack(llvm::Value* value);

        void reserve(std::size_t size);
    };

    struct PunctuationPackage {
        llvm::Value* watermark = nullptr;
        llvm::Value* isFinal = nullptr;
        llvm::Value* isHeartbeat = nullptr;

        static PunctuationPackage create(llvm::IRBuilder<>& builder, llvm::Value* watermark, llvm::Value* isFinal) {
            PunctuationPackage pkg;
            pkg.watermark = watermark;
            pkg.isFinal = isFinal;
            pkg.isHeartbeat = llvm::ConstantInt::getFalse(builder.getContext());
            return pkg;
        }

        static PunctuationPackage createFinal(llvm::IRBuilder<>& builder, llvm::Value* watermark) {
            PunctuationPackage pkg;
            pkg.watermark = watermark;
            pkg.isFinal = llvm::ConstantInt::getTrue(builder.getContext());
            pkg.isHeartbeat = llvm::ConstantInt::getFalse(builder.getContext());
            return pkg;
        }

        static PunctuationPackage createHeartbeat(llvm::IRBuilder<>& builder, llvm::Value* watermark) {
            PunctuationPackage pkg;
            pkg.watermark = watermark;
            pkg.isFinal = llvm::ConstantInt::getFalse(builder.getContext());
            pkg.isHeartbeat = llvm::ConstantInt::getTrue(builder.getContext());
            return pkg;
        }
    };
}  // namespace sierra::odyplan::v3::core::physical::data

#endif  // SIERRA_ODYPLAN_V3_CORE_PHYSICAL_DATA_DATA_PACKAGE_HPP