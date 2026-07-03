#ifndef SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORTTYPEREGISTRY_H
#define SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORTTYPEREGISTRY_H

#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>
#include "core/common/ident.h"

namespace sierra::odyplan::v2::core::physical::port {
    class PortTypeRegistry {
    public:
        using LLVMTypeFactory = std::function<llvm::Type*(llvm::LLVMContext&)>;

        void registerType(common::identifier::PortTypeId id, LLVMTypeFactory factory) {
            mTypes.emplace(id, std::move(factory));
        }

        [[nodiscard]] llvm::Type* resolveType(common::identifier::PortTypeId id, llvm::LLVMContext& ctx) const {
            const auto iter = mTypes.find(id);

            if (iter == mTypes.end()) {
                throw std::invalid_argument("PortTypeRegistry: Unknown PortTypeId given: " + std::to_string(id));
            }

            return iter->second(ctx);
        }
    private:
        std::unordered_map<common::identifier::PortTypeId, LLVMTypeFactory> mTypes;;
    };
}  // namespace sierra::odyplan::v2::core::physical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORTTYPEREGISTRY_H