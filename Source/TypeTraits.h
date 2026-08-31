#pragma once
#include <type_traits>

namespace jug
{

template<typename T, typename... TArgs>
constexpr bool IsAnyTypeOfV = std::disjunction_v<std::is_same<T, TArgs>...>;

template<typename T>
concept ArithmeticT = std::is_arithmetic_v<T>;

template<typename T>
concept PodT = std::is_trivial_v<T> && std::is_standard_layout_v<T>;
#define JUG_STATIC_ASSERT_POD(T, ...) static_assert(jug::PodT<T>, #T " must be a POD type")

}   // namespace jug