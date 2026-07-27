#ifndef SIERRA_ODYPLAN_V2_CORE_RUNTIME_THREAD_TASK_H
#define SIERRA_ODYPLAN_V2_CORE_RUNTIME_THREAD_TASK_H

#include <cstdint>
#include <functional>

namespace sierra::odyplan::v2::core::runtime::thread {
    using Task = std::function<void()>;
}

#endif //SIERRA_ODYPLAN_V2_CORE_RUNTIME_THREAD_TASK_H