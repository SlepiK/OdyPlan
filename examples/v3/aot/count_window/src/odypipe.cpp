#include <core/physical/types/value_type_registry.h>

extern "C" {
    void ody_init(const void* typeRegistry);
    void ody_main();
    void ody_shutdown();
}

int main() {
    auto typeRegistry = sierra::odyplan::v3::core::physical::types::ValueTypeRegistry::withBuiltins();
    ody_init(&typeRegistry);
    ody_main();
    ody_shutdown();
    return 0;
}