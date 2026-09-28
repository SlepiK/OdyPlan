#include <core/runtime/time_runtime.h>

namespace sierra::odyplan::v3::core::runtime::thread {
    namespace {
        const std::chrono::steady_clock::time_point& programClock() {
            static const std::chrono::steady_clock::time_point t = std::chrono::steady_clock::now();
            return t;
        }
    }

    extern "C" {
    shared::types::Timestamp rtTimeNow() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
    }

    shared::types::Timestamp rtProgramTimeNow() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::steady_clock::now() - programClock())
            .count();
    }
    }
}  // namespace sierra::odyplan::v3::core::runtime::thread