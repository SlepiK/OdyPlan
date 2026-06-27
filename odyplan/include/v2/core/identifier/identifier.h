#ifndef SIERRA_ODYPLAN_V2_CORE_IDENTIFIER_IDENTIFIER_H
#define SIERRA_ODYPLAN_V2_CORE_IDENTIFIER_IDENTIFIER_H
#include <cstddef>
#include <cstdint>
#include <functional>

namespace sierra::odyplan::v2::core::identifier {
    template <typename T>
    struct Identifier {
        std::size_t value;

        bool operator==(const Identifier&) const = default;

        operator std::size_t() const noexcept {
            return value;
        }

        struct Hash {
            std::size_t operator()(const Identifier& id) const noexcept {
                return std::hash<std::size_t>{}(id.value);
            }
        };
    };
}  // namespace sierra::odyplan::v2::core::identifier

#endif  // SIERRA_ODYPLAN_V2_CORE_IDENTIFIER_IDENTIFIER_H