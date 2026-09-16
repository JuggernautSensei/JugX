#pragma once

namespace jug
{

template<typename T, typename... Args>
constexpr bool AnyTypeOfV = std::disjunction_v<std::is_same<T, Args>...>;

template<typename T>
concept ArithmeticT = std::is_arithmetic_v<T>;

template<typename T>
concept PodT = std::is_trivial_v<T> && std::is_standard_layout_v<T>;

template<typename T>
concept CharT = AnyTypeOfV<T, char, char8_t, char16_t, char32_t, wchar_t>;

}   // namespace jug