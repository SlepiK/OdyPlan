#include <core/runtime/core_runtime.h>

#include "core/runtime/types/runtime_type_registry.h"

namespace sierra::odyplan::v3::core::runtime {
    extern "C" {
    void rtEnd() {
    }

    bool odyStop() {
        return atStop.load(std::memory_order_relaxed);
    }
    }
}  // namespace sierra::odyplan::v3::core::runtime