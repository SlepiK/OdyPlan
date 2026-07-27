#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_MEASURE_OPERATOR_RUNTIME_STATE_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_MEASURE_OPERATOR_RUNTIME_STATE_H

#include <string>
#include <atomic>
#include <chrono>
#include <mutex>

namespace sierra::odyplan::v2::core::runtime::measure {
    struct MeasureState {
        std::string label;
        std::atomic_uint64_t counter{0};
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::mutex printMutex;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_MEASURE_OPERATOR_RUNTIME_STATE_H