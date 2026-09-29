#include <memory>
#include <core/runtime/type_runtime.h>

namespace sierra::odyplan::v3::core::runtime {
    extern "C" {
    void *rtTypeRegistryRetain(const shared::types::ValueTypeId typeId,
                               void *ptr) {
        const auto* registry = &types::RuntimeTypeRegistry::instance();
        const auto* entry = registry->getEntry(typeId);

        if (entry == nullptr || !entry->isPointerType) {
            return ptr;
        }

        return entry->retainFunction(ptr);
    }

    void rtTypeRegistryRelease(const shared::types::ValueTypeId typeId,
                               void *ptr) {
        const auto* registry = &types::RuntimeTypeRegistry::instance();
        const auto* entry = registry->getEntry(typeId);

        if (entry == nullptr || !entry->isPointerType) {
            return;
        }

        entry->releaseFunction(ptr);
    }
    }
} // namespace sierra::odyplan::v3::core::runtime
