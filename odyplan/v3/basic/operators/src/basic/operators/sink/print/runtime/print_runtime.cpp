#include <basic/operators/sink/print/runtime/print_runtime.h>

namespace sierra::odyplan::v3::basic::operators::sink::print::runtime {
    extern "C" {
    void rtOdyPrintString(const char* data) {
        std::cout << data;
    }

    void rtOdyPrintDouble(const double value) {
        std::cout << value;
    }

    void rtOdyPrintFloat(const float value) {
        std::cout << value;
    }

    void rtOdyPrintInt(const std::int64_t value) {
        std::cout << value;
    }

    void rtOdyPrintSeperator() {
        std::cout << " ";
    }

    void rtOdyPrintTimeStamp(const std::int64_t value) {
        std::cout << value << "]";
    }

    void rtOdyPrintNewline() {
        std::cout << "\n";
        std::cout.flush();
    }
    }
}  // namespace sierra::odyplan::v3::basic::operators::sink::print::runtime