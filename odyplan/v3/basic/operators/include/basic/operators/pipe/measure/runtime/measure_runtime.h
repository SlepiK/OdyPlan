#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MESSURE_RUNTIME_MEASURE_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MESSURE_RUNTIME_MEASURE_RUNTIME_HPP

#include <print>
#include <cstdint>
#include <mutex>

#include <basic/operators/pipe/measure/details/measure_state.h>

constexpr std::uint64_t MEASUREMENT_AFTER_LINES_REPORT = 2;

extern "C" {
    inline void* rtCreateMeasure(const void* /*typeRegistry*/, const char* label) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto* state = new sierra::odyplan::v2::core::runtime::measure::MeasureState();
        state->label = label;
        return state;
    }

    inline void rtDestroyMeasure(void* measureState) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete static_cast<sierra::odyplan::v2::core::runtime::measure::MeasureState*>(measureState);
    }

    inline void rtMeasureTick(void* measureState) {
        auto* state = static_cast<sierra::odyplan::v2::core::runtime::measure::MeasureState*>(measureState);
        const auto n = state->counter.fetch_add(1, std::memory_order_relaxed) + 1;

        if (n % MEASUREMENT_AFTER_LINES_REPORT == 0) {
            const auto now = std::chrono::steady_clock::now();
            const std::scoped_lock lock(state->printMutex);
            const auto elapsed = std::chrono::duration<double>(now - state->start).count();
            std::println(stderr, "[{}] {} lines, {:.2f} lines/sec (win)", state->label, static_cast<unsigned long long>(n), elapsed > 0.0 ? (static_cast<double>(MEASUREMENT_AFTER_LINES_REPORT) / elapsed) : 0.0);
            std::fflush(stderr);
            state->start = now;
        }
    }

    inline void rtMeasureFinalReport(void* measureState) {
        auto* state = static_cast<sierra::odyplan::v2::core::runtime::measure::MeasureState*>(measureState);
        const auto total = state->counter.load(std::memory_order_relaxed);
        const auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - state->start).count();

        const std::scoped_lock lock(state->printMutex);
        std::println(stderr, "[{}] Total: {} lines in {:.3f} sec ({:.2f} lines/sec)", state->label, static_cast<unsigned long long>(total), elapsed, elapsed > 0.0 ? (static_cast<double>(total) / elapsed) : 0.0);
        std::fflush(stderr);
    }
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MESSURE_RUNTIME_MEASURE_RUNTIME_HPP