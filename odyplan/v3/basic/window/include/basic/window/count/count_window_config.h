#ifndef SIERRA_ODYPLAN_V2_COMMON_WINDOW_COUNT_WINDOWCONFIG_H
#define SIERRA_ODYPLAN_V2_COMMON_WINDOW_COUNT_WINDOWCONFIG_H

#include <cstdint>

namespace sierra::odyplan::v3::basic::window::count {
    struct CountWindowConfig {
        std::uint64_t size = 1;
        std::uint64_t slide = 1;
        std::uint64_t advance = 1;

        [[nodiscard]] bool usesSlideParam() const {
            return slide != 1;
        }
        
        [[nodiscard]] std::uint64_t getEffectiveAdvance() const {
            return usesSlideParam() ? slide : advance;
        }
    };
}  // namespace sierra::odyplan::v3::basic::window::count

#endif  // SIERRA_ODYPLAN_V2_COMMON_WINDOW_COUNT_WINDOWCONFIG_H