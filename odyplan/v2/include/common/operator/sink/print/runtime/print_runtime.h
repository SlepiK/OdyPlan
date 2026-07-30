#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_PRINT_OPERATOR_RUNTIME_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_PRINT_OPERATOR_RUNTIME_H
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

    inline void rtOdyPrintNewline() {
        std::cout << "\n";
        std::cout.flush();
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_PRINT_OPERATOR_RUNTIME_H