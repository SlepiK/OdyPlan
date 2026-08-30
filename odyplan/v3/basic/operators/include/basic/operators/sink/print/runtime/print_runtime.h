#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_RUNTIME_PRINT_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_RUNTIME_PRINT_RUNTIME_HPP
#include <cstdint>
#include <iostream>
#include <syncstream>

namespace sierra::odyplan::v3::basic::operators::sink::print::runtime {
    extern "C" {
    void rtOdyPrintString(const char* data);

    void rtOdyPrintDouble(double value);

    void rtOdyPrintFloat(float value);

    void rtOdyPrintInt(std::int64_t value);

    void rtOdyPrintSeperator();

    void rtOdyPrintTimeStamp(std::int64_t value);

    void rtOdyPrintWindowMetadata(std::int64_t start, std::int64_t end, bool hasEnd, bool isClosed);

    void rtOdyPrintNewline();

    void rtOdyPrintPunctuation(std::int64_t watermark, bool isFinal);
    }
}  // namespace sierra::odyplan::v3::basic::operators::sink::print::runtime

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_RUNTIME_PRINT_RUNTIME_HPP
