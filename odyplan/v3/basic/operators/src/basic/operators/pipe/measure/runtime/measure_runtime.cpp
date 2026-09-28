#include <basic/operators/pipe/measure/runtime/measure_runtime.h>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

namespace sierra::odyplan::v3::basic::operators::pipe::measure::runtime {
    extern "C" {
    void* rtCreateMeasure(const char* label) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto* state = new sierra::odyplan::v3::core::runtime::measure::MeasureState();
        state->label = label;
        return state;
    }

    void rtDestroyMeasure(void* measureState) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete static_cast<sierra::odyplan::v3::core::runtime::measure::MeasureState*>(
            measureState);
    }

    void rtMeasureTick(void* measureState) {
        auto* state =
            static_cast<sierra::odyplan::v3::core::runtime::measure::MeasureState*>(
                measureState);
        const auto n = state->counter.fetch_add(1, std::memory_order_relaxed) + 1;

        std::chrono::milliseconds snapshotMs{};
        std::chrono::steady_clock::duration snapshotCarry{};

        {
            while (state->accLock.test_and_set(std::memory_order_acquire)) {
            }

            const auto now = std::chrono::steady_clock::now();
            const auto elapsed = now - state->last;
            state->accuPureMsCarry += elapsed;
            const auto wholeMs = std::chrono::duration_cast<std::chrono::milliseconds>(state->accuPureMsCarry);
            state->accuPureMs += wholeMs;
            state->accuPureMsCarry -= wholeMs;
            state->last = now;

            snapshotMs = state->accuPureMs;
            snapshotCarry = state->accuPureMsCarry;

            state->accLock.clear(std::memory_order_release);
        }

        if (n % MEASUREMENT_AFTER_LINES_REPORT != 0) {
            return;
        }

        if (state->printLock.test_and_set(std::memory_order_acquire)) {
            return;
        }

        const double currentPureElapsedMs = static_cast<double>(snapshotMs.count()) +
            std::chrono::duration<double, std::milli>(snapshotCarry).count();

        const double currentLastPrintElapsedMs =
            static_cast<double>(state->lastPrintAccuPureMs.count()) +
            std::chrono::duration<double, std::milli>(state->lastPrintAccuPureMsCarry).count();

        const double printWindowMs = currentPureElapsedMs - currentLastPrintElapsedMs;

        spdlog::info(fmt::format("[{}] {} lines, {:.3f} lines/ms (window: {:.3f} ms), {:.3f} lines/ms (total: {:.3f} ms)",
        state->label,
            n,
            printWindowMs > 0.0 ? (static_cast<double>(MEASUREMENT_AFTER_LINES_REPORT) / printWindowMs) : 0.0, printWindowMs,
            currentPureElapsedMs > 0.0 ? (static_cast<double>(n) / currentPureElapsedMs) : 0.0,
            currentPureElapsedMs ));

        state->lastPrintAccuPureMs = snapshotMs;
        state->lastPrintAccuPureMsCarry = snapshotCarry;

        state->printLock.clear(std::memory_order_release);
    }

    void rtMeasureFinalReport(void* measureState) {
        auto* state =
            static_cast<sierra::odyplan::v3::core::runtime::measure::MeasureState*>(
                measureState);
        std::call_once(state->finalReportFlag, [state]() {
            const auto total = state->counter.load(std::memory_order_relaxed);

            while (state->accLock.test_and_set(std::memory_order_acquire)) {
            }

            const auto purePrintElapsedMs = static_cast<double>(state->accuPureMs.count()) + std::chrono::duration<double, std::milli>(state->accuPureMsCarry).count();

            state->accLock.clear(std::memory_order_release);

            while (state->printLock.test_and_set(std::memory_order_acquire)) {
            }


            spdlog::info(fmt::format("[{}] Total: {} lines in {:.3f} ms ({:.3f} lines/ms)",
                state->label,
                total,
                purePrintElapsedMs,
                purePrintElapsedMs > 0.0 ? (static_cast<double>(total) / purePrintElapsedMs) : 0.0));

            state->printLock.clear(std::memory_order_release);
        });
    }
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::measure::runtime