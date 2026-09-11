#pragma once
#include <array>

#include "Macro.h"
#include "Math.h"
#include "TypeTraits.h"
#include "Vector2I.h"
#include "Vector3I.h"

namespace jug
{

struct alignas(16) VECTOR4I
{
    using ValueT = int;

    JUG_MATH_API VECTOR4I() = default;

    JUG_MATH_API constexpr VECTOR4I(
        const int _x,
        const int _y,
        const int _z,
        const int _w)
        : e { _x, _y, _z, _w }
    {
    }

    JUG_MATH_API constexpr VECTOR4I(
        const VECTOR2I _xy,
        const int      _z,
        const int      _w)
        : e { _xy.e[0], _xy.e[1], _z, _w }
    {
    }

    JUG_MATH_API constexpr VECTOR4I(
        const VECTOR3I _xyz,
        const int      _w)
        : e { _xyz.e[0], _xyz.e[1], _xyz.e[2], _w }
    {
    }

    explicit JUG_MATH_API constexpr VECTOR4I(
        const int _value)
        : e { _value, _value, _value, _value }
    {
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR4I operator-() const
    {
        VECTOR4I v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] = -v.e[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR4I operator+(
        const VECTOR4I _other) const
    {
        VECTOR4I v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] += _other.e[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR4I operator-(
        const VECTOR4I _other) const
    {
        VECTOR4I v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] -= _other.e[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR4I operator*(
        const int _scalar) const
    {
        VECTOR4I v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] *= _scalar;
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR4I operator/(
        const int _scalar) const
    {
        VECTOR4I v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] /= _scalar;
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR4I operator*(
        const VECTOR4I _other) const
    {
        VECTOR4I v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] *= _other.e[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR4I operator/(
        const VECTOR4I _other) const
    {
        VECTOR4I v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] /= _other.e[i];
        }
        return v;
    }

    JUG_MATH_API constexpr VECTOR4I& operator+=(
        const VECTOR4I _other)
    {
        *this = *this + _other;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR4I& operator-=(
        const VECTOR4I _other)
    {
        *this = *this - _other;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR4I& operator*=(
        const int _scalar)
    {
        *this = *this * _scalar;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR4I& operator/=(
        const int _scalar)
    {
        *this = *this / _scalar;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR4I& operator*=(
        const VECTOR4I _other)
    {
        *this = *this * _other;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR4I& operator/=(
        const VECTOR4I _other)
    {
        *this = *this / _other;
        return *this;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool operator==(
        const VECTOR4I _other) const
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
        const VECTOR4I _other) const
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

    const static VECTOR4I kZero;
    const static VECTOR4I kOne;
    const static VECTOR4I kRight;
    const static VECTOR4I kUp;
    const static VECTOR4I kForward;
    const static VECTOR4I kUnitX;
    const static VECTOR4I kUnitY;
    const static VECTOR4I kUnitZ;
    const static VECTOR4I kUnitW;
    const static VECTOR4I kMax;
    const static VECTOR4I kMin;

    constexpr static size_t kDim = 4;

    JUG_DISABLE_ANON_WARNING_BEGIN
    union
    {
        struct
        {
            int x;
            int y;
            int z;
            int w;
        };
        ARRAY<int, 4> e;
    };
    JUG_DISABLE_ANON_WARNING_END
};

static_assert(PodT<VECTOR4I>, "VECTOR4I must be POD type.");

// =======================================================
//  Constants
// =======================================================

inline constexpr VECTOR4I VECTOR4I::kZero { 0 };
inline constexpr VECTOR4I VECTOR4I::kOne { 1 };
inline constexpr VECTOR4I VECTOR4I::kRight   = { 1, 0, 0, 0 };
inline constexpr VECTOR4I VECTOR4I::kUp      = { 0, 1, 0, 0 };
inline constexpr VECTOR4I VECTOR4I::kForward = { 0, 0, 1, 0 };
inline constexpr VECTOR4I VECTOR4I::kUnitX   = { 1, 0, 0, 0 };
inline constexpr VECTOR4I VECTOR4I::kUnitY   = { 0, 1, 0, 0 };
inline constexpr VECTOR4I VECTOR4I::kUnitZ   = { 0, 0, 1, 0 };
inline constexpr VECTOR4I VECTOR4I::kUnitW   = { 0, 0, 0, 1 };
inline constexpr VECTOR4I VECTOR4I::kMax { MathConstants<int>::kMax };
inline constexpr VECTOR4I VECTOR4I::kMin { MathConstants<int>::kMin };

template<>
struct MathConstants<VECTOR4I>
{
    constexpr static VECTOR4I kZero    = VECTOR4I::kZero;
    constexpr static VECTOR4I kOne     = VECTOR4I::kOne;
    constexpr static VECTOR4I kRight   = VECTOR4I::kRight;
    constexpr static VECTOR4I kUp      = VECTOR4I::kUp;
    constexpr static VECTOR4I kForward = VECTOR4I::kForward;
    constexpr static VECTOR4I kUnitX   = VECTOR4I::kUnitX;
    constexpr static VECTOR4I kUnitY   = VECTOR4I::kUnitY;
    constexpr static VECTOR4I kUnitZ   = VECTOR4I::kUnitZ;
    constexpr static VECTOR4I kUnitW   = VECTOR4I::kUnitW;
    constexpr static VECTOR4I kMax     = VECTOR4I::kMax;
    constexpr static VECTOR4I kMin     = VECTOR4I::kMin;
};

}   // namespace jug
