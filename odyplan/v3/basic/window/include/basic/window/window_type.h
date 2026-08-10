#ifndef SIERRA_ODYPLAN_V3_BASIC_WINDOW_WINDOW_TYPE_HPP
#define SIERRA_ODYPLAN_V3_BASIC_WINDOW_WINDOW_TYPE_HPP

#include <cstdint>

namespace sierra::odyplan::v3::basic::window {
    enum class WindowType : std::uint8_t {
        TUMBLING,
        SLIDING,
    };
}

#endif  // SIERRA_ODYPLAN_V3_BASIC_WINDOW_WINDOW_TYPE_HPP