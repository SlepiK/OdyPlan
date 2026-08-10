#include <core/runtime/time_runtime.h>

namespace sierra::odyplan::v3::core::runtime::thread {
    extern "C" {
        std::int64_t rtNow() {
#ifdef SIERRA_USER_PROGRAM_CLOCK
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - gClock).count();
#else
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
#endif
        }
    }
}