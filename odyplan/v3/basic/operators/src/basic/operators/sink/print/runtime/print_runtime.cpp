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

    void rtOdyPrintWindowMetadata(const std::int64_t start, const std::int64_t end, const bool hasEnd, const bool isClosed) {
        threadPrintBuffer += '[';
        threadPrintBuffer += std::to_string(start);
        threadPrintBuffer += '|';
        if (hasEnd) {
            threadPrintBuffer += std::to_string(end);
        } else {
            threadPrintBuffer += '-';
        }
        threadPrintBuffer += '|';
        if (isClosed) {
            threadPrintBuffer += "iC";
        } else {
            threadPrintBuffer += "nC";
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