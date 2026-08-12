#include "core/runtime/types/runtime_type_registry.h"

extern "C" {
    // NOLINTBEGIN(readability-identifier-naming)
    void ody_init(const void* runtimeTypeRegistry);
    void ody_main();
    void ody_shutdown();
    // NOLINTEND(readability-identifier-naming)
}

int main() {
    auto runtimeTypeRegistry = sierra::odyplan::v3::core::runtime::types::RuntimeTypeRegistry::withBuiltins();
    ody_init(&runtimeTypeRegistry);
    ody_main();
    ody_shutdown();
    return 0;
}