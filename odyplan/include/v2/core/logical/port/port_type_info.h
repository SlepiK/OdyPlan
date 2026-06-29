#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTTYPEINFO_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTTYPEINFO_H

#include <atomic>
#include "v2/core/identifier/identifier.h"

namespace sierra::odyplan::v2::core::logical::port {
    namespace detail {
        template  <typename T>
        [[nodiscard]] std::uint32_t getTypeId() {
            static const std::uint32_t id = []() {
                static std::atomic<std::uint32_t> next{0};
                return next.fetch_add(1, std::memory_order_relaxed);
            }();
            return id;
        }
    }

    struct PortTypeInfo {
        std::uint32_t id;
    };

    template <typename T>
    [[nodiscard]] constexpr port::PortTypeInfo getUniquePortTypeInfo() {
        return PortTypeInfo{.id = detail::getTypeId<T>()};
    }
}  // namespace sierra::odyplan::v2::core::logical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORTTYPEINFO_H