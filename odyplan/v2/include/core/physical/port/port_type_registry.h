#ifndef SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORTTYPEREGISTRY_H
#define SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORTTYPEREGISTRY_H

#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>
#include <vector>

#include "core/logical/port/port_descriptor.h"

namespace sierra::odyplan::v2::core::physical::port {
    class PortTypeRegistry {
    public:
        using LLVMTypeFactory = std::function<llvm::Type*(llvm::LLVMContext&)>;

        void registerType(logical::port::PortTypeId id, LLVMTypeFactory factory) {
            mTypes.emplace(id, std::move(factory));
        }

        [[nodiscard]] llvm::Type* resolveType(logical::port::PortTypeId id, llvm::LLVMContext& ctx) {
            auto iter = mTypes.find(id);

            if (iter == mTypes.end()) {
                throw std::invalid_argument("PortTypeRegistry: Unknown PortTypeId given: " + std::to_string(id));
            }

            return iter->second(ctx);
        }
    private:
        std::unordered_map<logical::port::PortTypeId, LLVMTypeFactory> mTypes;;
    };
}  // namespace sierra::odyplan::v2::core::physical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORTTYPEREGISTRY_H