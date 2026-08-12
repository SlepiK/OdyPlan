#include <basic/operators/sink/print/runtime/print_runtime.h>

namespace sierra::odyplan::v3::basic::operators::sink::print::runtime {
    namespace {
        thread_local std::string threadPrintBuffer;
    }

    extern "C" {
    void rtOdyPrintString(const char* data) {
        threadPrintBuffer += data;
    }

    void rtOdyPrintDouble(const double value) {
        threadPrintBuffer += std::to_string(value);
    }

    void rtOdyPrintFloat(const float value) {
        threadPrintBuffer += std::to_string(value);
    }

    void rtOdyPrintInt(const std::int64_t value) {
        threadPrintBuffer += std::to_string(value);
    }

    void rtOdyPrintSeperator() {
        threadPrintBuffer += ' ';
    }

    void rtOdyPrintTimeStamp(const std::int64_t value) {
        threadPrintBuffer += std::to_string(value);
        threadPrintBuffer += ']';
    }

    void rtOdyPrintNewline() {
        threadPrintBuffer += '\n';
        std::osyncstream(std::cout) << threadPrintBuffer;
        threadPrintBuffer.clear();
    }
    }
}  // namespace sierra::odyplan::v3::basic::operators::sink::print::runtime