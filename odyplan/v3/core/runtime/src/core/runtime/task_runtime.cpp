#include <core/runtime/task_runtime.h>

namespace sierra::odyplan::v3::core::runtime {
    extern "C" {
    IOTaskHandle rtIOTaskSpawn(void (*fn)()) {
        return gIoThreadPool.submitTask([fn] { fn(); });
    }

    void rtIOTaskJoin(const IOTaskHandle ioTaskHandle) {
        gIoThreadPool.joinWorker(ioTaskHandle);
    }

    void rtSleepMs(const shared::types::Timestamp ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
    }
}  // namespace sierra::odyplan::v3::core::runtime