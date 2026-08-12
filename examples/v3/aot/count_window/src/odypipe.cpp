#include <core/physical/types/value_type_registry.h>

#include "core/runtime/types/runtime_type_registry.h"

extern "C" {
    void ody_init(const void* runtimeTypeRegistry);
    void ody_main();
    void ody_shutdown();
}

int main() {
    auto runtimeTypeRegistry = sierra::odyplan::v3::core::runtime::types::RuntimeTypeRegistry::withBuiltins();
    ody_init(&runtimeTypeRegistry);
    ody_main();
    ody_shutdown();
    return 0;
}