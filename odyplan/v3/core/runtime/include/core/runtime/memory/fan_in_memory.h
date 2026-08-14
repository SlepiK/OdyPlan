#ifndef SIERRA_ODYPLAN_V3_CORE_RUNTIME_MEMORY_FAN_IN_MEMORY_HPP
#define SIERRA_ODYPLAN_V3_CORE_RUNTIME_MEMORY_FAN_IN_MEMORY_HPP
#include <cstddef>

namespace sierra::odyplan::v3::core::runtime::memory {
    extern "C" {
    void *rtFanInAlloc(std::size_t size);

    void rtFanInFree(void *ptr);
    }
}

#endif //SIERRA_ODYPLAN_V3_CORE_RUNTIME_MEMORY_FAN_IN_MEMORY_HPP