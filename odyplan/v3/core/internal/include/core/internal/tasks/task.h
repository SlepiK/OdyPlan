#ifndef SIERRA_ODYPLAN_V3_CORE_INTERNAL_TASKS_TASK_HPP
#define SIERRA_ODYPLAN_V3_CORE_INTERNAL_TASKS_TASK_HPP

#include <functional>

namespace sierra::odyplan::v3::core::internal::tasks {
    using Task = std::function<void()>;
}

#endif //SIERRA_ODYPLAN_V3_CORE_INTERNAL_TASKS_TASK_HPP