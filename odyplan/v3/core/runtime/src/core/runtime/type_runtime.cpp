#include <memory>
#include <core/runtime/type_runtime.h>

namespace {
    std::unique_ptr<sierra::odyplan::v3::core::runtime::types::RuntimeTypeRegistry> gRtTypeRegistry;
}

namespace sierra::odyplan::v3::core::runtime {
    extern "C" {
    void* odyCreateRtTypeRegistryExt() {
        if (!gRtTypeRegistry) {
            gRtTypeRegistry = std::make_unique<types::RuntimeTypeRegistry>(types::RuntimeTypeRegistry::withBuiltins());
        }
        return gRtTypeRegistry.get();
    }

    void odyDestroyRtTypeRegistryExt(types::RuntimeTypeRegistry*) {
        gRtTypeRegistry.reset();
    }

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
