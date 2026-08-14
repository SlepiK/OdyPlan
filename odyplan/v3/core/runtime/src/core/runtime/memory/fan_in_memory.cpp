#include <cstdlib>
#include <core/runtime/memory/fan_in_memory.h>

namespace sierra::odyplan::v3::core::runtime::memory {
    extern "C" {
    void *rtFanInAlloc(std::size_t size) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,cppcoreguidelines-no-malloc)
        void *ptr = std::malloc(size);
        if (ptr == nullptr) {
            std::abort();
        }
        return ptr;
    }

    void rtFanInFree(void *ptr) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,cppcoreguidelines-no-malloc)
        std::free(ptr);
    }
    }
}
