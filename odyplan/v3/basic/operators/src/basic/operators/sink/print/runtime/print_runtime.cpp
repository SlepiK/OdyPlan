#include <basic/operators/sink/print/runtime/print_runtime.h>

#include "core/physical/time/time_point.h"

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

    void rtOdyPrintTimeInterval(const std::int64_t startTs, const std::int64_t endTs, const bool endInfinity) {
        threadPrintBuffer += std::to_string(startTs);
        threadPrintBuffer += '|';
        if (endInfinity) {
            threadPrintBuffer += "inf";
        } else {
            threadPrintBuffer += std::to_string(endTs);
        }
        threadPrintBuffer += ']';
    }

    void rtOdyPrintNewline() {
        threadPrintBuffer += '\n';
        std::osyncstream(std::cout) << threadPrintBuffer;
        threadPrintBuffer.clear();
    }


    void rtOdyPrintPunctuation(const std::int64_t watermark, const bool isFinal) {
        threadPrintBuffer += "[PUNCTUATION ";
        if (isFinal) {
            threadPrintBuffer += "FINAL]";
        } else {
            threadPrintBuffer += "watermark=";
            threadPrintBuffer += std::to_string(watermark);
            threadPrintBuffer += ']';
        }
        rtOdyPrintNewline();
    }
    }
}  // namespace sierra::odyplan::v3::basic::operators::sink::print::runtime