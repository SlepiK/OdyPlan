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

    void rtOdyPrintTimeStamp(std::uint64_t value);

    void rtOdyPrintWindowMetadata(std::uint64_t start, std::uint64_t end, bool hasEnd);

    void rtOdyPrintNewline();

    void rtOdyPrintPunctuation(std::uint64_t watermark, bool isFinal);
    }
}  // namespace sierra::odyplan::v3::basic::operators::sink::print::runtime

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_RUNTIME_PRINT_RUNTIME_HPP
