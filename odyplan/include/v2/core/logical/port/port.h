#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORT_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORT_H

#include <tuple>

namespace sierra::odyplan::v2::core::logical::port {
    template <typename T>
    struct PortField {
        using ValueType = T;
        T value;
    };

    template <typename T>
    concept PortFieldType = requires { typename T::ValueType; } && requires(T t) { t.value; };

    struct NullPort {
        static constexpr std::size_t SIZE = 0;
    };

    template <PortFieldType... Ts>
    struct Port {
        using Fields = std::tuple<Ts...>;
    };

    template <typename T>
    concept PortType = requires { typename T::Fields; } || std::same_as<T, NullPort>;

    template <PortType... Ts>
    struct Ports {
        using PortEntries = std::tuple<Ts...>;
        static constexpr std::size_t SIZE = sizeof...(Ts);
    };

    template <typename T>
    concept PortsType = requires {
        typename T::PortEntries;
        { T::SIZE } -> std::convertible_to<std::size_t>;
    } || std::same_as<T, NullPort>;

    template <typename T>
    constexpr bool IS_NULL_PORT = std::same_as<T, NullPort>;
}  // namespace sierra::odyplan::v2::core::logical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORT_H