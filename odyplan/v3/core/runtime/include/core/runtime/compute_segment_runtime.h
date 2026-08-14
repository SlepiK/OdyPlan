#ifndef SIERRA_ODYPLAN_V3_CORE_RUNTIME_COMPUTE_SEGMENT_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_CORE_RUNTIME_COMPUTE_SEGMENT_RUNTIME_HPP

#include <core/runtime/task/compute_segment.h>
#include <atomic>
#include <mutex>

namespace sierra::odyplan::v3::core::runtime {
    extern "C" {
    void *rtComputeSegmentNullInit(const void *typeRegistry);

    void *rtComputeSegmentGetOrCreate(void **slot, void (*processFn)(void *));

    void rtComputeSegmentEnqueue(void *segmentHandle, void *payload);

    void rtComputeSegmentDestroyIfStarted(void *segmentHandle);
    }
}

#endif //SIERRA_ODYPLAN_V3_CORE_RUNTIME_COMPUTE_SEGMENT_RUNTIME_HPP