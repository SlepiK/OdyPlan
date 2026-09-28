#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_DETAILS_COUNT_WINDOW_OPTIONS_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_DETAILS_COUNT_WINDOW_OPTIONS_HPP

#include <cstdint>

namespace sierra::odyplan::v3::basic::operators::pipe::window::details {
    struct CountWindowOptions {
        std::size_t size = 1;
        std::size_t slide = 1;
        std::size_t advance = 1;

        [[nodiscard]] bool usesSlideParam() const {
            return slide != 1;
        }
        
        [[nodiscard]] std::size_t getEffectiveAdvance() const {
            return usesSlideParam() ? slide : advance;
        }
    };
}  // namespace sierra::odyplan::v3::basic::window::count

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_DETAILS_COUNT_WINDOW_OPTIONS_HPP