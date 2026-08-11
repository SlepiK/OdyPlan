#include <core/runtime/time_runtime.h>

namespace sierra::odyplan::v3::core::runtime::thread {
    namespace {
        const std::chrono::steady_clock::time_point PROGRAM_CLOCK =
            std::chrono::steady_clock::now();
    }
    extern "C" {
    std::int64_t rtTimeNow() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
    }
    std::int64_t rtProgramTimeNow() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - PROGRAM_CLOCK)
            .count();
    }
    }
}  // namespace sierra::odyplan::v3::core::runtime::thread