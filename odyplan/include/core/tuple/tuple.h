#ifndef SIERRA_ODYPLAN_V1_CORE_TUPLE_TUPLE_H
#define SIERRA_ODYPLAN_V1_CORE_TUPLE_TUPLE_H

#include "tuple_field.h"

#include <tuple>
#include <type_traits>

namespace sierra::odyplan::v1::core::tuple {
    template <typename T>
    struct IsStdTupleOfOdyfields : std::false_type {};

    template <typename... Ts>
    struct IsStdTupleOfOdyfields<std::tuple<Ts...>> : std::conjunction<IsTupleField<Ts>...> {};

    template <typename T>
    inline constexpr bool IS_STD_TUPLE_OF_ODYFIELDS_V =
        IsStdTupleOfOdyfields<std::remove_cvref_t<T>>::value;

    template <typename T>
    concept Tuple = IS_STD_TUPLE_OF_ODYFIELDS_V<T>;

    template <TupleField... Ts>
    using TupleT = std::tuple<Ts...>;
}  // namespace sierra::odyplan::v1::core::tuple

#endif  // SIERRA_ODYPLAN_V1_CORE_TUPLE_TUPLE_H