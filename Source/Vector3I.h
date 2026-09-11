#pragma once
#include <array>

#include "Macro.h"
#include "Math.h"
#include "TypeTraits.h"
#include "Vector2I.h"

namespace jug
{

struct VECTOR3I
{
    using ValueT = int;

    JUG_MATH_API VECTOR3I() = default;

    JUG_MATH_API constexpr VECTOR3I(
        const int _x,
        const int _y,
        const int _z)
        : e { _x, _y, _z }
    {
    }

    JUG_MATH_API constexpr VECTOR3I(
        const VECTOR2I _xy,
        const int      _z)
        : e { _xy.e[0], _xy.e[1], _z }
    {
    }

    explicit JUG_MATH_API constexpr VECTOR3I(
        const int _value)
        : e { _value, _value, _value }
    {
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3I operator-() const
    {
        VECTOR3I v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] = -v.e[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3I operator+(
        const VECTOR3I _other) const
    {
        VECTOR3I v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] += _other.e[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3I operator-(
        const VECTOR3I _other) const
    {
        VECTOR3I v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] -= _other.e[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3I operator*(
        const int _scalar) const
    {
        VECTOR3I v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] *= _scalar;
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3I operator/(
        const int _scalar) const
    {
        VECTOR3I v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] /= _scalar;
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3I operator*(
        const VECTOR3I _other) const
    {
        VECTOR3I v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] *= _other.e[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR3I operator/(
        const VECTOR3I _other) const
    {
        VECTOR3I v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] /= _other.e[i];
        }
        return v;
    }

    JUG_MATH_API constexpr VECTOR3I& operator+=(
        const VECTOR3I _other)
    {
        *this = *this + _other;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR3I& operator-=(
        const VECTOR3I _other)
    {
        *this = *this - _other;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR3I& operator*=(
        const int _scalar)
    {
        *this = *this * _scalar;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR3I& operator/=(
        const int _scalar)
    {
        *this = *this / _scalar;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR3I& operator*=(
        const VECTOR3I _other)
    {
        *this = *this * _other;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR3I& operator/=(
        const VECTOR3I _other)
    {
        *this = *this / _other;
        return *this;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool operator==(
        const VECTOR3I _other) const
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
        const VECTOR3I _other) const
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

    const static VECTOR3I kZero;
    const static VECTOR3I kOne;
    const static VECTOR3I kRight;
    const static VECTOR3I kUp;
    const static VECTOR3I kForward;
    const static VECTOR3I kUnitX;
    const static VECTOR3I kUnitY;
    const static VECTOR3I kUnitZ;
    const static VECTOR3I kMax;
    const static VECTOR3I kMin;

    constexpr static size_t kDim = 3;

    JUG_DISABLE_ANON_WARNING_BEGIN
    union
    {
        struct
        {
            int x;
            int y;
            int z;
        };
        ARRAY<int, 3> e;
    };
    JUG_DISABLE_ANON_WARNING_END
};

static_assert(PodT<VECTOR3I>, "VECTOR3I must be POD type.");

// =======================================================
//  Constants
// =======================================================

inline constexpr VECTOR3I VECTOR3I::kZero { 0 };
inline constexpr VECTOR3I VECTOR3I::kOne { 1 };
inline constexpr VECTOR3I VECTOR3I::kRight   = { 1, 0, 0 };
inline constexpr VECTOR3I VECTOR3I::kUp      = { 0, 1, 0 };
inline constexpr VECTOR3I VECTOR3I::kForward = { 0, 0, 1 };
inline constexpr VECTOR3I VECTOR3I::kUnitX   = { 1, 0, 0 };
inline constexpr VECTOR3I VECTOR3I::kUnitY   = { 0, 1, 0 };
inline constexpr VECTOR3I VECTOR3I::kUnitZ   = { 0, 0, 1 };
inline constexpr VECTOR3I VECTOR3I::kMax { MathConstants<int>::kMax };
inline constexpr VECTOR3I VECTOR3I::kMin { MathConstants<int>::kMin };

template<>
struct MathConstants<VECTOR3I>
{
    constexpr static VECTOR3I kZero    = VECTOR3I::kZero;
    constexpr static VECTOR3I kOne     = VECTOR3I::kOne;
    constexpr static VECTOR3I kRight   = VECTOR3I::kRight;
    constexpr static VECTOR3I kUp      = VECTOR3I::kUp;
    constexpr static VECTOR3I kForward = VECTOR3I::kForward;
    constexpr static VECTOR3I kUnitX   = VECTOR3I::kUnitX;
    constexpr static VECTOR3I kUnitY   = VECTOR3I::kUnitY;
    constexpr static VECTOR3I kUnitZ   = VECTOR3I::kUnitZ;
    constexpr static VECTOR3I kMax     = VECTOR3I::kMax;
    constexpr static VECTOR3I kMin     = VECTOR3I::kMin;
};

}   // namespace jug
