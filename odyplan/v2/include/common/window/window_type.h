#ifndef SIERRA_ODYPLAN_V2_COMMON_WINDOW_WINDOWCONFIG_H
#define SIERRA_ODYPLAN_V2_COMMON_WINDOW_WINDOWCONFIG_H

namespace sierra::odyplan::v2::common::window {
    enum class WindowType : std::uint8_t {
        TUMBLING,
        SLIDING,
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_WINDOW_WINDOWCONFIG_H