#include <basic/expression/runtime/expression_runtime.h>
#include <core/runtime/types/runtime_type_registry.h>

namespace sierra::odyplan::v3::basic::expression::runtime {
    extern "C" {
        bool rtTypeEqual(const core::shared::types::ValueTypeId typeId, const void* left, const void* right) {
            auto *leftByte = reinterpret_cast<const std::byte*>(left);
            auto *rightByte = reinterpret_cast<const std::byte*>(right);

            const auto *registry = &core::runtime::types::RuntimeTypeRegistry::instance();
            auto* cmpFn = registry->getEqualFnPtr(typeId);

            return cmpFn(leftByte, rightByte);
        }
    }
}
