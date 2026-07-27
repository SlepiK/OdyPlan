#ifndef SIERRA_ODYPLAN_V2_CORE_INTERNAL_THREAD_TASK_H
#define SIERRA_ODYPLAN_V2_CORE_INTERNAL_THREAD_TASK_H

#include <cstdint>
#include <functional>

namespace sierra::odyplan::v2::core::internal::thread {
    using Task = std::function<void()>;
}

#endif //SIERRA_ODYPLAN_V2_CORE_INTERNAL_THREAD_TASK_H