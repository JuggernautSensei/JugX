#pragma once
#include "Macros.h"
#include "Math.h"
#include "MathMacros.h"
#include "Typedef.h"
#include "TypeTraits.h"

namespace jug
{

struct VECTOR2
{
    using ValueT = float;

    JUG_MATH_API VECTOR2() = default;

    JUG_MATH_API constexpr VECTOR2(
        const float _x,
        const float _y)
        : e { _x, _y }
    {
    }

    // Broadcast
    explicit JUG_MATH_API constexpr VECTOR2(
        const float _value)
        : e { _value, _value }
    {
    }

    // =======================================================
    //  Operators
    // =======================================================

    [[nodiscard]] JUG_MATH_API constexpr VECTOR2 operator-() const
    {
        VECTOR2 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] = -v.e[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR2 operator+(
        const VECTOR2 _other) const
    {
        VECTOR2 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] += _other.e[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR2 operator-(
        const VECTOR2 _other) const
    {
        VECTOR2 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] -= _other.e[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR2 operator*(
        const float _scalar) const
    {
        VECTOR2 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] *= _scalar;
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR2 operator/(
        const float _scalar) const
    {
        VECTOR2 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] /= _scalar;
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR2 operator*(
        const VECTOR2 _other) const
    {
        VECTOR2 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] *= _other.e[i];
        }
        return v;
    }

    [[nodiscard]] JUG_MATH_API constexpr VECTOR2 operator/(
        const VECTOR2 _other) const
    {
        VECTOR2 v = *this;
        for (size_t i = 0; i < kDim; ++i)
        {
            v.e[i] /= _other.e[i];
        }
        return v;
    }

    // =======================================================
    //  Assignment
    // =======================================================

    JUG_MATH_API constexpr VECTOR2& operator+=(
        const VECTOR2 _other)
    {
        *this = *this + _other;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR2& operator-=(
        const VECTOR2 _other)
    {
        *this = *this - _other;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR2& operator*=(
        const float _scalar)
    {
        *this = *this * _scalar;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR2& operator/=(
        const float _scalar)
    {
        *this = *this / _scalar;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR2& operator*=(
        const VECTOR2 _other)
    {
        *this = *this * _other;
        return *this;
    }

    JUG_MATH_API constexpr VECTOR2& operator/=(
        const VECTOR2 _other)
    {
        *this = *this / _other;
        return *this;
    }

    [[nodiscard]] JUG_MATH_API constexpr bool operator==(
        const VECTOR2 _other) const
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
        const VECTOR2 _other) const
    {
        return !(*this == _other);
    }

    // =======================================================
    //  Access
    // =======================================================

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

    const static VECTOR2 kZero;
    const static VECTOR2 kOne;
    const static VECTOR2 kRight;
    const static VECTOR2 kUp;
    const static VECTOR2 kUnitX;
    const static VECTOR2 kUnitY;
    const static VECTOR2 kMax;
    const static VECTOR2 kMin;

    constexpr static size_t kDim = 2;

    JUG_MATH_DISABLE_ANON_WARNING_BEGIN
    union
    {
        struct
        {
            float x;
            float y;
        };
        ARRAY<float, 2> e;
    };
    JUG_MATH_DISABLE_ANON_WARNING_END
};

JUG_STATIC_ASSERT_POD(VECTOR2);

// =======================================================
//  Constants
// =======================================================

inline constexpr VECTOR2 VECTOR2::kZero { 0.f };
inline constexpr VECTOR2 VECTOR2::kOne { 1.f };
inline constexpr VECTOR2 VECTOR2::kRight = { 1.f, 0.f };
inline constexpr VECTOR2 VECTOR2::kUp    = { 0.f, 1.f };
inline constexpr VECTOR2 VECTOR2::kUnitX = { 1.f, 0.f };
inline constexpr VECTOR2 VECTOR2::kUnitY = { 0.f, 1.f };
inline constexpr VECTOR2 VECTOR2::kMax { MathConstants<float>::kMax };
inline constexpr VECTOR2 VECTOR2::kMin { MathConstants<float>::kMin };

template<>
struct MathConstants<VECTOR2>
{
    constexpr static VECTOR2 kZero  = VECTOR2::kZero;
    constexpr static VECTOR2 kOne   = VECTOR2::kOne;
    constexpr static VECTOR2 kRight = VECTOR2::kRight;
    constexpr static VECTOR2 kUp    = VECTOR2::kUp;
    constexpr static VECTOR2 kUnitX = VECTOR2::kUnitX;
    constexpr static VECTOR2 kUnitY = VECTOR2::kUnitY;
    constexpr static VECTOR2 kMax   = VECTOR2::kMax;
    constexpr static VECTOR2 kMin   = VECTOR2::kMin;
};

}   // namespace jug
