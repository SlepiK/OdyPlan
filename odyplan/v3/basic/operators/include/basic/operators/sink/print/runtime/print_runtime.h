#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_RUNTIME_PRINT_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_RUNTIME_PRINT_RUNTIME_HPP
#include <iostream>
#include <cstdint>

extern "C" {
    inline void rtOdyPrintString(const char* data) {
        std::cout << data;
    }

    inline void rtOdyPrintDouble(const double value) {
        std::cout << value;
    }

    inline void rtOdyPrintFloat(const float value) {
        std::cout << value;
    }

    inline void rtOdyPrintInt(const std::int64_t value) {
        std::cout << value;
    }

    inline void rtOdyPrintSeperator() {
        std::cout << " ";
    }

    inline void rtOdyPrintTimeStamp(const std::int64_t value) {
        std::cout << value << "]";
    }

    inline void rtOdyPrintNewline() {
        std::cout << "\n";
        std::cout.flush();
    }
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_RUNTIME_PRINT_RUNTIME_HPP