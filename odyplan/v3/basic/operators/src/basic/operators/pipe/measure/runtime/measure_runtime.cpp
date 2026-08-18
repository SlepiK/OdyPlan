#include <basic/operators/pipe/measure/runtime/measure_runtime.h>

namespace sierra::odyplan::v3::basic::operators::pipe::measure::runtime {
    extern "C" {
    void* rtCreateMeasure(const void* /*typeRegistry*/, const char* label) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto* state = new sierra::odyplan::v3::core::runtime::measure::details::MeasureState();
        state->label = label;
        return state;
    }

    void rtDestroyMeasure(void* measureState) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete static_cast<sierra::odyplan::v3::core::runtime::measure::details::MeasureState*>(
            measureState);
    }

    void rtMeasureTick(void* measureState) {
        auto* state =
            static_cast<sierra::odyplan::v3::core::runtime::measure::details::MeasureState*>(
                measureState);
        const auto n = state->counter.fetch_add(1, std::memory_order_relaxed) + 1;

        if (n % MEASUREMENT_AFTER_LINES_REPORT == 0) {
            const auto now = std::chrono::steady_clock::now();
            const std::scoped_lock lock(state->printMutex);
            const auto elapsed = std::chrono::duration<double>(now - state->start).count();
            std::println(stdout,
                         "[{}] {} lines, {:.2f} lines/sec (win)",
                         state->label,
                         static_cast<unsigned long long>(n),
                         elapsed > 0.0
                             ? (static_cast<double>(MEASUREMENT_AFTER_LINES_REPORT) / elapsed)
                             : 0.0);
            std::fflush(stdout);
            state->start = now;
        }
    }

    void rtMeasureFinalReport(void* measureState) {
        auto* state =
            static_cast<sierra::odyplan::v3::core::runtime::measure::details::MeasureState*>(
                measureState);
        const auto total = state->counter.load(std::memory_order_relaxed);
        const auto elapsed =
            std::chrono::duration<double>(std::chrono::steady_clock::now() - state->start).count();

        const std::scoped_lock lock(state->printMutex);
        std::println(stdout,
                     "[{}] Total: {} lines in {:.3f} sec ({:.2f} lines/sec)",
                     state->label,
                     static_cast<unsigned long long>(total),
                     elapsed,
                     elapsed > 0.0 ? (static_cast<double>(total) / elapsed) : 0.0);
        std::fflush(stdout);
    }
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::measure::runtime