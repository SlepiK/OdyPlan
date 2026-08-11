#ifndef SIERRA_ODYPLAN_V3_CORE_RUNTIME_CORE_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_CORE_RUNTIME_CORE_RUNTIME_HPP

#include <atomic>
#include <core/runtime/task_runtime.h>

inline std::atomic atStop{false};

namespace sierra::odyplan::v3::core::runtime {
    extern "C" {
    void rtEnd();

    bool odyStop();
    }
}  // namespace sierra::odyplan::v3::core::runtime

#endif  // SIERRA_ODYPLAN_V3_CORE_RUNTIME_CORE_RUNTIME_HPP