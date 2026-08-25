#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MESSURE_RUNTIME_DETAILS_MEASURE_STATE_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MESSURE_RUNTIME_DETAILS_MEASURE_STATE_HPP

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>

namespace sierra::odyplan::v3::core::runtime::measure::details {
    struct MeasureState {
        std::string label;
        std::atomic_uint64_t counter{0};
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point last = std::chrono::steady_clock::now();
        std::chrono::milliseconds accuPureMs{0};
        std::chrono::steady_clock::duration accuPureMsCarry{std::chrono::steady_clock::duration::zero()};
        std::atomic_flag printLock = ATOMIC_FLAG_INIT;
        std::once_flag finalReportFlag;

    };
}  // namespace sierra::odyplan::v2::core::runtime::measure::details

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MESSURE_RUNTIME_DETAIL_MEASURE_STATE_HPP