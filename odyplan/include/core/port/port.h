#ifndef SIERRA_ODYPLAN_V1_CORE_PORT_PORT_H
#define SIERRA_ODYPLAN_V1_CORE_PORT_PORT_H

#include <tuple>

namespace sierra::odyplan::v1::core::port {
    template <typename T>
    struct PortField {
        using value_type = T;
        T value;
    };

    template <typename T>
    concept PortFieldType = requires {
        typename T::value_type;
    } && requires(T t) {
        t.value;
    };

    struct NullPort {};

    template <PortFieldType... Ts>
    struct Port {
        using Fields = std::tuple<Ts...>;
    };

    template <typename T>
    concept PortType = requires {
        typename T::Fields;
    } || std::same_as<T, NullPort>;

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
}  // namespace sierra::odyplan::v1::core::port

#endif  // SIERRA_ODYPLAN_V1_CORE_PORT_PORT_H