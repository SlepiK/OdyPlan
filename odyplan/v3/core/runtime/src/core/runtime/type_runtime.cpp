#include <memory>
#include <core/runtime/type_runtime.h>

namespace sierra::odyplan::v3::core::runtime {
    extern "C" {
    void *rtTypeRegistryRetain(const shared::types::ValueTypeId typeId,
                               void *ptr) {
        return core::runtime::types::RuntimeTypeRegistry::instance().retain(typeId, ptr);
    }

    void rtTypeRegistryRelease(const shared::types::ValueTypeId typeId,
                               void *ptr) {
        core::runtime::types::RuntimeTypeRegistry::instance().release(typeId, ptr);
    }
    }
} // namespace sierra::odyplan::v3::core::runtime
