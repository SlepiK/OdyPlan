#include "core/runtime/types/runtime_type_registry.h"

extern "C" {
    // NOLINTBEGIN(readability-identifier-naming)
    void ody_init();
    void ody_main();
    void ody_shutdown();
    // NOLINTEND(readability-identifier-naming)
}

int main() {
    ody_init();
    ody_main();
    ody_shutdown();
    return 0;
}