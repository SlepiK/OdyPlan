#ifndef SIERRA_ODYPLAN_V2_CORE_RUNTIME_HELPER_MEASURE_H
#define SIERRA_ODYPLAN_V2_CORE_RUNTIME_HELPER_MEASURE_H

#include <core/runtime/measure/measure_state.h>

constexpr uint64_t MEASUREMENT_AFTER_LINES_REPORT = 2;

extern "C" {
    inline void* rtCreateMeasure(const void* /*typeRegistry*/, const char* label) {
        auto* state = new sierra::odyplan::v2::core::runtime::measure::MeasureState();
        state->label = label;
        return state;
    }

    inline void rtDestroyMeasure(void* measureState) {
        delete static_cast<sierra::odyplan::v2::core::runtime::measure::MeasureState*>(measureState);
    }

    inline void rtMeasureTick(void* measureState) {
        auto* state = static_cast<sierra::odyplan::v2::core::runtime::measure::MeasureState*>(measureState);
        const auto n = state->counter.fetch_add(1, std::memory_order_relaxed) + 1;

        if (n % MEASUREMENT_AFTER_LINES_REPORT == 0) {
            const auto now = std::chrono::steady_clock::now();
            const std::lock_guard<std::mutex> lock(state->printMutex);
            const auto elapsed = std::chrono::duration<double>(now - state->start).count();
            std::fprintf(stderr, "[%s] %llu lines, %.2f lines/sec (win)\n", state->label.c_str(), static_cast<unsigned long long>(n), elapsed > 0.0 ? MEASUREMENT_AFTER_LINES_REPORT : 0.0);
            state->start = now;
        }
    }

    inline void rtMeasureFinalReport(void* measureState) {
        auto* state = static_cast<sierra::odyplan::v2::core::runtime::measure::MeasureState*>(measureState);
        const auto total = state->counter.load(std::memory_order_relaxed);
        const auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - state->start).count();

        const std::lock_guard<std::mutex> lock(state->printMutex);
        std::fprintf(stderr, "[%s] Total: %llu lines in %.3f sec (%.2f lines/sec)\n", state->label.c_str(), static_cast<unsigned long long>(total), elapsed, elapsed > 0.0 ? static_cast<double>(total) / elapsed : 0.0);
    }
}

#endif //SIERRA_ODYPLAN_V2_CORE_RUNTIME_HELPER_MEASURE_H