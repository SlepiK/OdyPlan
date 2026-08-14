#include <core/runtime/type_runtime.h>

namespace sierra::odyplan::v3::core::runtime {
    extern "C" {
    void *rtTypeRegistryRetain(const void *registry,
                               const shared::types::ValueTypeId typeId,
                               void *ptr) {
        using types::RuntimeTypeRegistry;
        return static_cast<const RuntimeTypeRegistry *>(registry)->retain(typeId, ptr);
    }

    void rtTypeRegistryRelease(const void *registry,
                               const shared::types::ValueTypeId typeId,
                               void *ptr) {
        using types::RuntimeTypeRegistry;
        static_cast<const RuntimeTypeRegistry *>(registry)->release(typeId, ptr);
    }
    }
} // namespace sierra::odyplan::v3::core::runtime
