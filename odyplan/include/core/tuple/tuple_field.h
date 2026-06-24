#ifndef SIERRA_ODYPLAN_V1_CORE_TUPLE_TUPLEFIELD_H
#define SIERRA_ODYPLAN_V1_CORE_TUPLE_TUPLEFIELD_H

#include <type_traits>

namespace sierra::odyplan::v1::core::tuple {
    template <typename T, auto Name>
    struct TupleFieldT {
        T value;
    };

    template <typename T>
    struct IsTupleField : std::false_type {};

    template <typename T, auto Name>
    struct IsTupleField<TupleFieldT<T, Name>> : std::true_type {};

    template <typename T>
    inline constexpr bool IS_TUPLE_FIELD_V = IsTupleField<std::remove_cvref_t<T>>::value;

    template <typename T>
    concept TupleField = IS_TUPLE_FIELD_V<T>;
}  // namespace sierra::odyplan::v1::core::tuple

#endif  // SIERRA_ODYPLAN_V1_CORE_TUPLE_TUPLEFIELD_H