#include <core/runtime/core_runtime.h>

#include "core/runtime/types/runtime_type_registry.h"

namespace sierra::odyplan::v3::core::runtime {
    extern "C" {
    void rtEnd() {
    }

    bool odyStop() {
        return atStop.load(std::memory_order_relaxed);
    }

    void ody_runtime_ext_init() {
        const auto runtimeTypeRegistry = sierra::odyplan::v3::core::runtime::types::RuntimeTypeRegistry::withBuiltins();
        ody_init(&runtimeTypeRegistry);
    }

    void ody_runtime_ext_main() {
        ody_main();
    }

    void ody_runtime_ext_shutdown() {
        ody_shutdown();
    }
    }
}  // namespace sierra::odyplan::v3::core::runtime