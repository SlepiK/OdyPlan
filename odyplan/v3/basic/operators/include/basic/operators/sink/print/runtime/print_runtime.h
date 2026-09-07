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

    void rtOdyPrintTimeInterval(std::int64_t startTs, std::int64_t endTs, bool endInfinity);

    void rtOdyPrintNewline();

    void rtOdyPrintPunctuation(std::int64_t watermark, bool isFinal);
    }
}  // namespace sierra::odyplan::v3::basic::operators::sink::print::runtime

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_RUNTIME_PRINT_RUNTIME_HPP
