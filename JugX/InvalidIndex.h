#pragma once
#include "Math.h"

namespace jug
{

template<std::integral T>
inline constexpr T kInvalidIndexValue = Max<T>();

struct InvalidIndexType
{
    struct Tag
    {
    };

    explicit constexpr InvalidIndexType(
        const Tag)
    {
    }

    template<std::integral T>
    [[nodiscard]] /* implicit */ constexpr operator T() const
    {
        return kInvalidIndexValue<T>;
    }
};

inline constexpr InvalidIndexType kInvalidIndex { InvalidIndexType::Tag {} };

// ===========================================
//  Comparison
// ===========================================

template<std::integral T>
[[nodiscard]] constexpr bool operator==(
    const T _value,
    const InvalidIndexType)
{
    return _value == kInvalidIndexValue<T>;
}

// ===========================================
//  Utils
// ===========================================

template<std::integral T>
[[nodiscard]] constexpr bool IsValid(
    const T _value)
{
    return _value != kInvalidIndex;
}

}   // namespace jug

