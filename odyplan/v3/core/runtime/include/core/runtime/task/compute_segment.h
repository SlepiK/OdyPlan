#ifndef SIERRA_ODYPLAN_V3_CORE_RUNTIME_TASK_COMPUTE_SEGMENT_HPP
#define SIERRA_ODYPLAN_V3_CORE_RUNTIME_TASK_COMPUTE_SEGMENT_HPP

#include <core/runtime/task/bonded_queue.h>

namespace sierra::odyplan::v3::core::runtime::task {
    struct ComputeSegment {
        BoundedQueue queue;
        std::thread worker;
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_RUNTIME_TASK_COMPUTE_SEGMENT_HPP