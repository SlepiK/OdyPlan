#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MESSURE_RUNTIME_MEASURE_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MESSURE_RUNTIME_MEASURE_RUNTIME_HPP

#include <basic/operators/pipe/measure/runtime/details/measure_state.h>
#include <cstdint>
#include <mutex>
#include <print>

constexpr std::uint64_t MEASUREMENT_AFTER_LINES_REPORT = 2;

namespace sierra::odyplan::v3::basic::operators::pipe::measure::runtime {
    extern "C" {
    void* rtCreateMeasure(const void* /*typeRegistry*/, const char* label);

    void rtDestroyMeasure(void* measureState);

    void rtMeasureTick(void* measureState);

    void rtMeasureFinalReport(void* measureState);
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::measure::runtime

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MESSURE_RUNTIME_MEASURE_RUNTIME_HPP