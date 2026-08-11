#include <core/runtime/core_runtime.h>

namespace sierra::odyplan::v3::core::runtime {
    extern "C" {
    void rtEnd() {
#ifdef SIERRA_RUNTIME_IO
        sierra::odyplan::v3::basic::operators::sink::write::runtime::rtIOSinkFlushAll();
#endif
    }

    bool odyStop() {
        return atStop.load(std::memory_order_relaxed);
    }
    }
}  // namespace sierra::odyplan::v3::core::runtime