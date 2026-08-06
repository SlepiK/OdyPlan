#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MESSURE_RUNTIME_DETAILS_MEASURE_STATE_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MESSURE_RUNTIME_DETAILS_MEASURE_STATE_HPP

#include <string>
#include <atomic>
#include <chrono>
#include <mutex>

namespace sierra::odyplan::v2::core::runtime::measure::details {
    struct MeasureState {
        std::string label;
        std::atomic_uint64_t counter{0};
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::mutex printMutex;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MESSURE_RUNTIME_DETAIL_MEASURE_STATE_HPP