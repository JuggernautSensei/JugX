#pragma once

namespace jug
{

template<typename T, typename... Args>
constexpr bool AnyTypeOfV = std::disjunction_v<std::is_same<std::remove_cv_t<T>, Args>...>;

template<typename T>
concept ArithmeticT = std::is_arithmetic_v<T>;

template<typename T>
concept IntegerWithoutBool = std::is_integral_v<T> && !std::is_same_v<std::remove_cv_t<T>, bool>;

template<typename T>
concept PodT = std::is_trivial_v<T> && std::is_standard_layout_v<T>;

template<typename T>
concept CharT = AnyTypeOfV<T, char, char8_t, char16_t, char32_t, wchar_t>;

template<typename T>
concept PointerT = std::is_pointer_v<T>;

}   // namespace jug