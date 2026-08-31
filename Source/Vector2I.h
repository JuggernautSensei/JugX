#pragma once
#include <array>

#include "Macros.h"
#include "Math.h"
#include "TypeTraits.h"

namespace jug
{

struct VECTOR2I
{
    using ValueT = int;

    JUG_MATH_API VECTOR2I() = default;

    JUG_MATH_API constexpr VECTOR2I(
        const int _x,
        const int _y)
        : e { _x, _y }
    {
    }

    explicit JUG_MATH_API constexpr VECTOR2I(
        const int _value)
        : e { _value, _value }
    {
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR2I operator-() const
    {
        VECTOR2I v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] = -v.e[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR2I operator+(
        const VECTOR2I _other) const
    {
        VECTOR2I v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] += _other.e[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR2I operator-(
        const VECTOR2I _other) const
    {
        VECTOR2I v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] -= _other.e[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR2I operator*(
        const int _scalar) const
    {
        VECTOR2I v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] *= _scalar;
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR2I operator/(
        const int _scalar) const
    {
        VECTOR2I v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] /= _scalar;
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR2I operator*(
        const VECTOR2I _other) const
    {
        VECTOR2I v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] *= _other.e[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR2I operator/(
        const VECTOR2I _other) const
    {
        VECTOR2I v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] /= _other.e[i];
        }
        return v;
    }

    JUG_MATH_API constexpr VECTOR2I& operator+=(
        const VECTOR2I _other)
    {
        *this = *this + _other;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR2I& operator-=(
        const VECTOR2I _other)
    {
        *this = *this - _other;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR2I& operator*=(
        const int _scalar)
    {
        *this = *this * _scalar;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR2I& operator/=(
        const int _scalar)
    {
        *this = *this / _scalar;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR2I& operator*=(
        const VECTOR2I _other)
    {
        *this = *this * _other;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR2I& operator/=(
        const VECTOR2I _other)
    {
        *this = *this / _other;
        return *this;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool operator==(
        const VECTOR2I _other) const
    {
        for (size_t i = 0; i < kDim; ++i)
        {
            if (e[i] != _other.e[i])   // NOLINT
            {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool operator!=(
        const VECTOR2I _other) const
    {
        return !(*this == _other);
    }

    [[nodiscard]] JUG_MATH_API constexpr ValueT& operator[](
        const size_t _index)
    {
        return e[_index];
    }

    [[nodiscard]] JUG_MATH_API constexpr const ValueT& operator[](
        const size_t _index) const
    {
        return e[_index];
    }

    [[nodiscard]] JUG_MATH_API constexpr ValueT* GetPtr()
    {
        return e.data();
    }

    [[nodiscard]] JUG_MATH_API constexpr const ValueT* GetPtr() const
    {
        return e.data();
    }

    // =======================================================
    //  Fields
    // =======================================================

    const static VECTOR2I kZero;
    const static VECTOR2I kOne;
    const static VECTOR2I kRight;
    const static VECTOR2I kUp;
    const static VECTOR2I kUnitX;
    const static VECTOR2I kUnitY;
    const static VECTOR2I kMax;
    const static VECTOR2I kMin;

    constexpr static size_t kDim = 2;

    JUG_MATH_DISABLE_ANON_WARNING_BEGIN
    union
    {
        struct
        {
            int x;
            int y;
        };
        ARRAY<int, 2> e;
    };
    JUG_MATH_DISABLE_ANON_WARNING_END
};

JUG_STATIC_ASSERT_POD(VECTOR2I);

// =======================================================
//  Constants
// =======================================================

inline constexpr VECTOR2I VECTOR2I::kZero { 0 };
inline constexpr VECTOR2I VECTOR2I::kOne { 1 };
inline constexpr VECTOR2I VECTOR2I::kRight = { 1, 0 };
inline constexpr VECTOR2I VECTOR2I::kUp    = { 0, 1 };
inline constexpr VECTOR2I VECTOR2I::kUnitX = { 1, 0 };
inline constexpr VECTOR2I VECTOR2I::kUnitY = { 0, 1 };
inline constexpr VECTOR2I VECTOR2I::kMax { MathConstants<int>::kMax };
inline constexpr VECTOR2I VECTOR2I::kMin { MathConstants<int>::kMin };

template<>
struct MathConstants<VECTOR2I>
{
    constexpr static VECTOR2I kZero  = VECTOR2I::kZero;
    constexpr static VECTOR2I kOne   = VECTOR2I::kOne;
    constexpr static VECTOR2I kRight = VECTOR2I::kRight;
    constexpr static VECTOR2I kUp    = VECTOR2I::kUp;
    constexpr static VECTOR2I kUnitX = VECTOR2I::kUnitX;
    constexpr static VECTOR2I kUnitY = VECTOR2I::kUnitY;
    constexpr static VECTOR2I kMax   = VECTOR2I::kMax;
    constexpr static VECTOR2I kMin   = VECTOR2I::kMin;
};

}   // namespace jug
